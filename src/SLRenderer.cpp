#include "SLRenderer.h"

void SLRenderer::render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	resourceManager.updatePushConstants();
	Renderer::render(fbo, cmdBuffer);
}

void SLRenderer::setupSubpasses()
{
	auto& subpass = resourceManager.getSubpass();
	subpass.addColorAttachment(getColorAttachmentSlot());
	subpass.setDepthAttachment(getDepthAttachmentSlot());
	addSubpassGraph(subpass);
}

void SLRenderer::initResources(uint32_t, uint32_t)
{
	resourceManager.init(getDevice(), getRenderPass(), *camera);
}
