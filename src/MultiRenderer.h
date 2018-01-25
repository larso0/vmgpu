#ifndef VMGPU_SORTLASTRENDERER_H
#define VMGPU_SORTLASTRENDERER_H

#include "Renderer.h"
#include "subpasses/CompositingSubpass.h"
#include "subpasses/MeshSubpass.h"
#include "SecondaryRenderer.h"
#include <bpView/Window.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/Semaphore.h>
#include <bp/Event.h>
#include <vector>
#include <utility>

class MultiRenderer : public Renderer
{
public:
	using Strategy = SecondaryRenderer::Strategy;

	MultiRenderer() :
		strategy{Strategy::SORT_LAST},
		instance{nullptr},
		mesh{nullptr},
		meshNode{&sceneRoot},
		cameraNode{&sceneRoot},
		camera{&cameraNode},
		deviceCount{2},
		frameCmdBuffer{VK_NULL_HANDLE},
		queue{nullptr},
		resized{false} {}

	void setStrategy(Strategy strategy) { MultiRenderer::strategy = strategy; }
	void setDeviceCount(uint32_t count) { deviceCount = count; }
	void init(bp::Instance& instance, uint32_t width, uint32_t height,
		  bpScene::Mesh& mesh) override;
	void setColor(uint32_t deviceIndex, const glm::vec3& color);
	void render() override;
	void update(float delta) override;

	bool shouldClose() override;
	uint32_t getDeviceCount() const { return deviceCount; }

private:
	Strategy strategy;
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	bpScene::Node sceneRoot, meshNode, cameraNode;
	bpScene::Camera camera;

	uint32_t deviceCount;
	std::vector<bp::Device> devices;
	std::vector<SecondaryRenderer> secondaryRenderers;
	std::vector<MeshSubpass> subpasses;

	bp::Texture renderColorAttachment;
	bp::Texture renderDepthAttachment;
	bp::RenderPass renderPass;

	std::vector<bp::Texture> compositingColorSources;
	std::vector<void*> mappedColorDst;
	std::vector<bp::Texture> compositingDepthSources;
	std::vector<void*> mappedDepthDst;

	bpView::Window window;
	bp::Swapchain swapchain;
	bp::Texture depthAttachment;
	CompositingSubpass compositingSubpass;
	bp::RenderPass compositingRenderPass;

	bp::CommandPool cmdPool;
	bp::Semaphore frameCompleteSem;
	VkCommandBuffer frameCmdBuffer;
	bp::Queue* queue;
	bool resized;

	bool isDeviceChosen(VkPhysicalDevice device);
	std::vector<VkRect2D> calcululateSubRendererAreas(uint32_t width, uint32_t height);
	std::vector<std::pair<uint32_t, uint32_t>> calculateMeshPortions();
	void resize(uint32_t w, uint32_t h);
	void recordCopy();
	void recordRender();
	void recordCompositing();
};


#endif
