#include "SingleRenderer.h"

using namespace bp;
using namespace bpUtil;
using namespace bpScene;

void SingleRenderer::init(Instance& instance, uint32_t width, uint32_t height,
			  Mesh& mesh, Scene& scene)
{
	SingleRenderer::instance = &instance;
	SingleRenderer::mesh = &mesh;
	SingleRenderer::scene = &scene;

	window.init(instance, width, height, "vmgpu");
	width = window.getWidth();
	height = window.getHeight();

	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.features.geometryShader = VK_TRUE;
	requirements.surface = window;
	requirements.extensions.push_back("VK_KHR_swapchain");
	device.init(instance, requirements);
	queue = &device.getGraphicsQueue();

	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.0});
	swapchain.init(device, window, width, height, false);

	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);

	meshSubpass.addColorAttachment(swapchain);

	connect(window.resizeEvent, swapchain, &Swapchain::resize);
	connect(swapchain.resizeEvent, [this](uint32_t w, uint32_t h){
		depthAttachment.resize(w, h);
		renderPass.setRenderArea({{}, {w, h}});
		float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		SingleRenderer::scene->camera.setPerspectiveProjection(glm::radians(60.f),
								       aspectRatio, 0.01f, 1000.f);
	});

	renderPass.addSubpassGraph(meshSubpass);
	renderPass.setRenderArea({{}, {width, height}});
	renderPass.init(width, height);

	cmdPool.init(device.getGraphicsQueue());
	renderCompleteSem.init(device);

	cmdBuffer = cmdPool.allocateCommandBuffer();
}

void SingleRenderer::render()
{
	window.handleEvents();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	renderPass.render(cmdBuffer);
	vkEndCommandBuffer(cmdBuffer);

	queue->submit({{swapchain.getImageAvailableSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		      {cmdBuffer}, {renderCompleteSem});
	queue->waitIdle();

	swapchain.present(renderCompleteSem);
}

bool SingleRenderer::shouldClose()
{
	return static_cast<bool>(glfwWindowShouldClose(window.getHandle()));
}