#include "SortLastRenderer.h"
#include <algorithm>
#include <future>

using namespace bp;
using namespace std;

void SortLastRenderer::init(Instance& instance, uint32_t width, uint32_t height,
			    bpScene::Mesh& mesh)
{
	SortLastRenderer::instance = &instance;
	SortLastRenderer::mesh = &mesh;

	devices.reserve(deviceCount);
	subRenderers.reserve(deviceCount);
	subpasses.reserve(deviceCount);

	//Setup scene
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	sceneRoot.update();
	camera.update();

	//Initialize window
	window.init(instance, width, height, "vmgpu");
	width = window.getWidth();
	height = window.getHeight();

	//Find device to create the swapchain for the window
	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.surface = window;
	requirements.extensions.push_back("VK_KHR_swapchain");
	devices.emplace_back(instance, requirements);

	//Setup attachments and subpass for composition
	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.0});
	swapchain.init(devices[0], window, width, height, false);

	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(devices[0], width, height);

	compositionSubpass.setDepthTestEnabled(true);
	compositionSubpass.addColorAttachment(swapchain);
	compositionSubpass.setDepthAttachment(depthAttachment);
	compositionRenderPass.addSubpassGraph(compositionSubpass);

	//Find the remaining devices
	requirements.surface = VK_NULL_HANDLE;
	requirements.extensions.clear();
	auto availableDevices = queryDevices(instance, requirements);

	for (uint32_t i = 1; i < deviceCount; i++)
	{
		VkPhysicalDevice selected = devices[0];
		for (VkPhysicalDevice device : availableDevices)
		{
			if (isDeviceChosen(device)) continue;
			else break;
		}

		devices.emplace_back(selected, requirements);
	}

	uint32_t meshPortion = mesh.getElementCount() / deviceCount;
	meshPortion -= meshPortion % 3;
	uint32_t firstMeshPortion = meshPortion
				    + (mesh.getElementCount() - meshPortion * deviceCount);
	uint32_t offset = 0;

	//Setup sub renderers
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		uint32_t elementCount = i == 0 ? firstMeshPortion : meshPortion;

		subpasses.emplace_back();
		subpasses[i].setScene(mesh, offset, elementCount, meshNode, camera);
		offset += elementCount;

		subRenderers.emplace_back();
		subRenderers[i].init(devices[i], devices[0], width, height, subpasses[i]);

		compositionSubpass.addTexture({{}, {width, height}},
					      subRenderers[i].getTargetColorTexture(),
					      subRenderers[i].getTargetDepthTexture());
	}

	//Initialize resources for compositing
	compositionRenderPass.init(width, height);
	compositionRenderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(devices[0].getGraphicsQueue());
	compositionPassCompleteSem.init(devices[0]);
	compositionCmdBuffer = cmdPool.allocateCommandBuffer();

	//Delegate for handling resizing of resources
	connect(window.resizeEvent, [this](uint32_t w, uint32_t h){
		swapchain.resize(w, h);
		depthAttachment.resize(w, h);
		compositionRenderPass.resize(w, h);
		compositionRenderPass.setRenderArea({{}, {w, h}});
		float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
		for (auto i = 0; i < deviceCount; i++)
		{
			subRenderers[i].resize(w, h);
			compositionSubpass.resizeTextureResources(i, {{}, {w, h}});
		}
	});
}

void SortLastRenderer::setColor(uint32_t deviceIndex, const glm::vec3& color)
{
	subpasses[deviceIndex].setColor(color);
}

void SortLastRenderer::render()
{
	window.handleEvents();

	vector<future<void>> futures;

	//Render
	for (auto& r : subRenderers)
		futures.push_back(async(launch::async, [&r]{ r.render(); }));
	for (auto& f : futures) f.wait();

	//Copy to GPU responsible for compositing
	futures.clear();
	for (auto& r : subRenderers)
		futures.push_back(async(launch::async, [&r]{ r.copyToTarget(); }));
	for (auto& f : futures) f.wait();
	for (auto& r : subRenderers) r.targetUnmap();

	//Compositing
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(compositionCmdBuffer, &beginInfo);
	compositionRenderPass.render(compositionCmdBuffer);
	vkEndCommandBuffer(compositionCmdBuffer);

	cmdPool.submit({{swapchain.getPresentSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		       {compositionCmdBuffer}, {compositionPassCompleteSem});
	cmdPool.waitQueueIdle();

	swapchain.present(compositionPassCompleteSem);
}

void SortLastRenderer::update(float delta)
{
	meshNode.rotate(delta, {0.f, 1.f, 0.f});
	meshNode.update();
}

bool SortLastRenderer::shouldClose()
{
	return static_cast<bool>(glfwWindowShouldClose(window.getHandle()));
}

bool SortLastRenderer::isDeviceChosen(VkPhysicalDevice device)
{
	auto found = find_if(devices.begin(), devices.end(),
			     [device](Device& other) -> bool{
				     return other.getPhysicalHandle() == device;
			     });
	return found != devices.end();
}