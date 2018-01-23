#include "BorderlessSubRenderer.h"

using namespace bp;

void BorderlessSubRenderer::init(VkInstance instance, VkPhysicalDevice physicalDevice,
				 Subpass& subpass, const VkRect2D& area)
{
	BorderlessSubRenderer::subpass = &subpass;

	window.init(instance, area.extent.width, area.extent.height, "subrenderer", nullptr,
		    FlagSet<Window::Flags>() << Window::Flags::FLOATING
					     << Window::Flags::VISIBLE
					     << Window::Flags::AUTO_ICONIFY);
	window.setPosition(area.offset.x, area.offset.y);

	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.surface = window;
	requirements.extensions.push_back("VK_KHR_swapchain");
	device.init(physicalDevice, requirements);
	queue = &device.getGraphicsQueue();

	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	swapchain.init(device, window, area.extent.width, area.extent.height, false);
	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			     area.extent.width, area.extent.height);

	subpass.addColorAttachment(swapchain);
	subpass.setDepthAttachment(depthAttachment);

	renderPass.addSubpassGraph(subpass);
	renderPass.setRenderArea({{}, {area.extent}});
	renderPass.init(area.extent.width, area.extent.height);

	renderCompleteSem.init(device);
	cmdPool.init(device.getGraphicsQueue());
	cmdBuffer = cmdPool.allocateCommandBuffer();

	//Delegate for handling resizing of resources
	connect(window.resizeEvent, [this](uint32_t w, uint32_t h){
		swapchain.resize(w, h);
		depthAttachment.resize(w, h);
		renderPass.resize(w, h);
	});
}

void BorderlessSubRenderer::render()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	renderPass.render(cmdBuffer);
	vkEndCommandBuffer(cmdBuffer);

	queue->submit({{swapchain.getImageAvailableSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		      {cmdBuffer}, {renderCompleteSem});
	queue->waitIdle();
}

void BorderlessSubRenderer::present()
{
	swapchain.present(renderCompleteSem);
}