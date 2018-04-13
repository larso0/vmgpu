#ifndef VMGPU_SFRENDERER_H
#define VMGPU_SFRENDERER_H

#include "ResourceManager.h"
#include <bpMulti/SortFirstRenderer.h>
#include <bpScene/Camera.h>

class SFRenderer : public bpMulti::SortFirstRenderer
{
public:
	SFRenderer() : camera{nullptr} {}

	void setCamera(bpScene::Camera& camera) { SFRenderer::camera = &camera; }
	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
	ResourceManager& getResourceManager() { return resourceManager; }
private:
	bpScene::Camera* camera;
	ResourceManager resourceManager;

	void setupSubpasses() override;
	void initResources(uint32_t, uint32_t) override;
	void updateClipTransform();
};


#endif
