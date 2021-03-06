#include "SFRenderer.h"

void SFRenderer::render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	resourceManager.updatePushConstants();
	Renderer::render(fbo, cmdBuffer);
}

void SFRenderer::setupSubpasses()
{
	auto& subpass = resourceManager.getSubpass();
	subpass.addColorAttachment(getColorAttachmentSlot());
	subpass.setDepthAttachment(getDepthAttachmentSlot());
	addSubpassGraph(subpass);
}

void SFRenderer::initResources(uint32_t, uint32_t)
{
	resourceManager.init(getDevice(), getRenderPass(), *camera, generateNormals);
	bpUtil::connect(contributionChangedEvent, *this, &SFRenderer::updateClipTransform);
	updateClipTransform();
}

void SFRenderer::updateClipTransform()
{
	resourceManager.setClipTransform(getContributionClipTransform());
}
