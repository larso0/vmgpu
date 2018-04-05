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
	void updateClipTransform();
};


#endif
