#ifndef VMGPU_SLRENDERER_H
#define VMGPU_SLRENDERER_H

#include "ResourceManager.h"
#include <bpMulti/SortLastRenderer.h>
#include <bpScene/Camera.h>

class SLRenderer : public bpMulti::SortLastRenderer
{
public:
	SLRenderer() : camera{nullptr} {}

	void setCamera(bpScene::Camera& camera) { SLRenderer::camera = &camera; }
	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
	ResourceManager& getResourceManager() { return resourceManager; }
	void increaseWorkload(float) override {}
	void decreaseWorkload(float) override {}

private:
	bpScene::Camera* camera;
	ResourceManager resourceManager;

	void setupSubpasses() override;
	void initResources(uint32_t, uint32_t) override;
};


#endif
