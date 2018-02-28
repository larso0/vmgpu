#ifndef VMGPU_SORTFIRSTBORDERLESSRENDERER_H
#define VMGPU_SORTFIRSTBORDERLESSRENDERER_H

#include "Renderer.h"
#include "subpasses/MeshSubpass.h"
#include "BorderlessSubRenderer.h"
#include <utility>

class SortFirstBorderlessRenderer : public Renderer
{
public:
	SortFirstBorderlessRenderer() :
		instance{nullptr},
		mesh{nullptr},
		scene{nullptr},
		deviceCount{2} {}

	void setDeviceCount(uint32_t deviceCount)
	{
		SortFirstBorderlessRenderer::deviceCount = deviceCount;
	}
	void init(bp::Instance& instance, uint32_t width, uint32_t height,
		  bpScene::Mesh& mesh, Scene& scene) override;
	void render() override;
	bool shouldClose() override;

private:
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	Scene* scene;

	uint32_t deviceCount;
	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<BorderlessSubRenderer> subRenderers;
	std::vector<MeshSubpass> subpasses;
	std::vector<VkRect2D> areas;

	std::vector<VkRect2D> calcululateSubRendererAreas(uint32_t width, uint32_t height);
};


#endif
