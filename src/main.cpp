#include <bpView/bpView.h>
#include <bpView/Window.h>
#include <bp/Instance.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/RenderPass.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include "subpasses/MeshSubpass.h"

using namespace bp;
using namespace bpScene;
using namespace std;
namespace po = boost::program_options;

enum class Mode
{
	Single,
	SortFirst,
	SortFirstBorderlessWindowCompositing,
	SortLast
};

struct Options
{
	Mode mode;
	string objPath;
	uint32_t width, height;
};

Options parseOptions(int argc, char** argv)
{
	Options result;

	po::options_description options;
	options.add_options()
		("help", "print help message")
		("resolution,r", po::value<string>()->default_value("1024x768"),
		 "resolution of window: <width>x<height>")
		("mode,m", po::value<string>()->default_value("single"),
		 "mode for rendering: single, sort-first, or sort-last")
		("borderless-window-compositing",
		 "use borderless windows for compositing with sort-last method")
		("file,f", po::value<string>(), "obj file to load 3D mesh from");
	po::variables_map arguments;
	po::store(po::parse_command_line(argc, argv, options), arguments);

	if (arguments.count("help"))
	{
		cout << options << endl;
		throw 1;
	}

	{
		string res = arguments["resolution"].as<string>();
		int n = sscanf(res.c_str(), "%ux%u", &result.width, &result.height);
		if (n != 2)
		{
			cerr << "Could not parse resolution, "
			     << "using fallback resolution of 1024x768." << endl;
			result.width = 1024;
			result.height = 768;
		}
	}

	{
		string m = arguments["mode"].as<string>();
		if (m == "single") result.mode = Mode::Single;
		else if (m == "sort-first")
		{
			if (arguments.count("borderless-window-compositing"))
				result.mode = Mode::SortFirstBorderlessWindowCompositing;
			else
				result.mode = Mode::SortFirst;
		} else if (m == "sort-last")
		{
			result.mode = Mode::SortLast;
		} else
		{
			cerr << "Unknown rendering mode, using single as fallback mode." << endl;
			result.mode = Mode::Single;
		}
	}

	if (arguments.count("file"))
	{
		result.objPath = arguments["file"].as<string>();
	} else
	{
		cerr << "Can't load mesh, no obj file was specified.\n";
		throw 2;
	}

	return result;
}

int main(int argc, char** argv)
{
	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

	if (options.mode != Mode::Single)
	{
		cerr << "Mode not implemented." << endl;
		return 3;
	}

	Mesh mesh;
	mesh.loadObj(options.objPath, FlagSet<Mesh::LoadFlags>()
		<< Mesh::LoadFlags::POSITION
		<< Mesh::LoadFlags::NORMAL);
	Node sceneRoot, meshNode{&sceneRoot}, cameraNode{&sceneRoot};
	Camera camera{&cameraNode};

	float aspectRatio = static_cast<float>(options.width) / static_cast<float>(options.height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	sceneRoot.update();
	camera.update();

	bpView::init();

#ifdef NDEBUG
	bool debugEnabled = false;
#else
	bool debugEnabled = true;
#endif

	Instance instance;

	instance.enableExtensions(bpView::requiredInstanceExtensions.begin(),
				  bpView::requiredInstanceExtensions.end());
	instance.init(debugEnabled);

	auto print = [](const string& s) { cout << s << endl; };
	auto printErr = [](const string& s) { cerr << s << endl; };

	connect(instance.infoEvent, print);
	connect(instance.warningEvent, printErr);
	connect(instance.errorEvent, printErr);
	connect(bpView::errorEvent, instance.errorEvent);

	static const char* SWAPCHAIN_EXTENSION = "VK_KHR_swapchain";

	Window window{instance, options.width, options.height, "vmgpu", nullptr,
		      FlagSet<Window::Flags>() << Window::Flags::VISIBLE
					       << Window::Flags::DECORATED
					       << Window::Flags::AUTO_ICONIFY};

	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.features.geometryShader = VK_TRUE;
	requirements.surface = window.getSurface();
	requirements.extensions.push_back(SWAPCHAIN_EXTENSION);

	Device device{instance, requirements};

	Swapchain swapchain;
	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	swapchain.init(&device, window, options.width, options.height, false);

	//Single GPU rendering

	DepthAttachment depthAttachment;
	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(&device, options.width, options.height);

	MeshSubpass subpass;
	subpass.setScene(&mesh, 0, mesh.getElementCount(), &meshNode, &camera);
	subpass.addColorAttachment(&swapchain);
	subpass.setDepthAttachment(&depthAttachment);

	RenderPass renderPass;
	renderPass.addSubpassGraph(&subpass);
	renderPass.setRenderArea({{}, {options.width, options.height}});
	renderPass.init(options.width, options.height);

	Queue& graphicsQueue = device.getGraphicsQueue();

	VkCommandPool cmdPool;
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = graphicsQueue.getQueueFamilyIndex();
	VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");

	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBufferInfo = {};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.commandPool = cmdPool;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	result = vkAllocateCommandBuffers(device, &cmdBufferInfo, &cmdBuffer);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate command buffer.");

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkSemaphore renderCompleteSem;
	VkSemaphoreCreateInfo semInfo = {};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	result = vkCreateSemaphore(device, &semInfo, nullptr, &renderCompleteSem);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create render complete semaphore.");

	VkSemaphore presentSem = swapchain.getPresentSemaphore();
	VkPipelineStageFlags waitStages = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSem;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentSem;
	submitInfo.pWaitDstStageMask = &waitStages;

	double seconds = glfwGetTime();
	double frametimeAccumulator = seconds;
	unsigned frameCounter = 0;
	while (!glfwWindowShouldClose(window.getHandle()))
	{
		vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
		renderPass.render(cmdBuffer);
		vkEndCommandBuffer(cmdBuffer);
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		swapchain.present(renderCompleteSem);

		bpView::waitEvents();
		window.handleEvents();

		double time = glfwGetTime();
		float delta = time - seconds;
		seconds = time;
		if (++frameCounter % 50 == 0)
		{
			double diff = time - frametimeAccumulator;
			frametimeAccumulator = time;
			double fps = 50.0 / diff;
			cout << '\r' << setprecision(4) << fps << "FPS";
		}
	}

	vkDestroySemaphore(device, renderCompleteSem, nullptr);
	vkDestroyCommandPool(device, cmdPool, nullptr);

	return 0;
}