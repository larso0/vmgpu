#ifndef VMGPU_SLRENDERER_H
#define VMGPU_SLRENDERER_H

#include "ResourceManager.h"
#include <bpMulti/SortLastRenderer.h>
#include <bpScene/Camera.h>

class SLRenderer : public bpMulti::SortLastRenderer
{
	SLRenderer() : camera{nullptr} {}

	void setCamera(bpScene::Camera& camera) { SLRenderer::camera = &camera; }
	unsigned addMesh(bpScene::Mesh& mesh) { resourceManager.addMesh(mesh); }
	void addEntity(unsigned meshId, bpScene::Node& node)
	{
		resourceManager.addEntity(meshId, node);
	}

	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;

private:
	bpScene::Camera* camera;
	ResourceManager resourceManager;

	void setupSubpasses() override;
	void initResources(uint32_t, uint32_t) override;
};


#endif
