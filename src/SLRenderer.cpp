#include "SLRenderer.h"

unsigned SLRenderer::addMesh(bpScene::Mesh& mesh, uint32_t offset, uint32_t count)
{
	resourceManager.addMesh(mesh, offset, count);
}


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
