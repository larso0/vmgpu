#include "SingleRenderer.h"

using namespace bp;
using namespace bpScene;

void SingleRenderer::init(Instance& instance, uint32_t width, uint32_t height,
			  Mesh& mesh)
{
	SingleRenderer::instance = &instance;
	SingleRenderer::mesh = &mesh;

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	sceneRoot.update();
	camera.update();

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

	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.0});
	swapchain.init(device, window, width, height, false);

	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);

	meshSubpass.setScene(mesh, 0, mesh.getElementCount(), meshNode, camera);
	meshSubpass.addColorAttachment(swapchain);
	meshSubpass.setDepthAttachment(depthAttachment);

	renderPass.addSubpassGraph(meshSubpass);
	renderPass.setRenderArea({{}, {width, height}});
	renderPass.init(width, height);

	connect(window.resizeEvent, [this](uint32_t w, uint32_t h){
		swapchain.resize(w, h);
		depthAttachment.resize(w, h);
		renderPass.resize(w, h);
		renderPass.setRenderArea({{}, {w, h}});
		float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	});

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

	cmdPool.submit({{swapchain.getPresentSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
			{cmdBuffer}, {renderCompleteSem});
	cmdPool.waitQueueIdle();

	swapchain.present(renderCompleteSem);
}

void SingleRenderer::update(float delta)
{
	meshNode.rotate(delta, {0.f, 1.f, 0.f});
	meshNode.update();
}

bool SingleRenderer::shouldClose()
{
	return static_cast<bool>(glfwWindowShouldClose(window.getHandle()));
}