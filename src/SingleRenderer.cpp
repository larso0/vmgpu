#include "SingleRenderer.h"

using namespace bp;
using namespace bpScene;

void SingleRenderer::init(NotNull<Instance> instance, uint32_t width, uint32_t height,
			  NotNull<Mesh> mesh, NotNull<Node> meshNode, NotNull<Camera> camera)
{
	SingleRenderer::instance = instance;
	SingleRenderer::mesh = mesh;
	SingleRenderer::meshNode = meshNode;
	SingleRenderer::camera = camera;

	window.init(*instance, width, height, "vmgpu");

	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.features.geometryShader = VK_TRUE;
	requirements.surface = window;
	requirements.extensions.push_back("VK_KHR_swapchain");
	device.init(*instance, requirements);

	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.0});
	swapchain.init(&device, window, width, height, false);

	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(&device, width, height);

	meshSubpass.setScene(mesh, 0, mesh->getElementCount(), meshNode, camera);
	meshSubpass.addColorAttachment(&swapchain);
	meshSubpass.setDepthAttachment(&depthAttachment);

	renderPass.addSubpassGraph(&meshSubpass);
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

	cmdPool.execute({{swapchain.getPresentSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
			{cmdBuffer}, {renderCompleteSem});
	swapchain.present(renderCompleteSem);
}

bool SingleRenderer::shouldClose()
{
	return static_cast<bool>(glfwWindowShouldClose(window.getHandle()));
}