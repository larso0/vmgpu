#ifndef VMGPU_SORTLASTRENDERER_H
#define VMGPU_SORTLASTRENDERER_H

#include "Renderer.h"
#include "subpasses/CompositionSubpass.h"
#include "subpasses/MeshSubpass.h"
#include "SortLastSubRenderer.h"
#include <bpView/Window.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/DepthAttachment.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/Semaphore.h>
#include <vector>

class SortLastRenderer : public Renderer
{
public:
	SortLastRenderer() :
		instance{nullptr},
		mesh{nullptr},
		meshNode{&sceneRoot},
		cameraNode{&sceneRoot},
		camera{&cameraNode},
		deviceCount{2},
		compositionCmdBuffer{VK_NULL_HANDLE} {}


	void setDeviceCount(uint32_t count) { deviceCount = count; }
	void init(bp::Instance& instance, uint32_t width, uint32_t height,
		  bpScene::Mesh& mesh) override;
	void setColor(uint32_t deviceIndex, const glm::vec3& color);
	void render() override;
	void update(float delta) override;

	bool shouldClose() override;

private:
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	bpScene::Node sceneRoot, meshNode, cameraNode;
	bpScene::Camera camera;

	bp::Window window;

	uint32_t deviceCount;
	std::vector<bp::Device> devices;
	std::vector<SortLastSubRenderer> subRenderers;
	std::vector<MeshSubpass> subpasses;

	bp::Swapchain swapchain;
	bp::DepthAttachment depthAttachment;
	CompositionSubpass compositionSubpass;
	bp::RenderPass compositionRenderPass;
	bp::CommandPool cmdPool;
	bp::Semaphore compositionPassCompleteSem;
	VkCommandBuffer compositionCmdBuffer;

	bool isDeviceChosen(VkPhysicalDevice device);
};


#endif
