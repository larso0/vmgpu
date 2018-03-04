#ifndef VMGPU_SORTLASTRENDERER_H
#define VMGPU_SORTLASTRENDERER_H

#include "Renderer.h"
#include "subpasses/CompositingSubpass.h"
#include "subpasses/SFBenchSubpass.h"
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
#include <unordered_map>
#include <mutex>

class MultiRenderer : public Renderer
{
public:
	using Strategy = SecondaryRenderer::Strategy;

	MultiRenderer() :
		strategy{Strategy::SORT_FIRST},
		instance{nullptr},
		mesh{nullptr},
		scene{nullptr},
		deviceCount{2},
		frameCmdBuffer{VK_NULL_HANDLE},
		queue{nullptr},
		resized{false} {}

	void setStrategy(Strategy strategy) { MultiRenderer::strategy = strategy; }
	void setDeviceCount(uint32_t count) { deviceCount = count; }
	void init(bp::Instance& instance, uint32_t width, uint32_t height,
		  bpScene::Mesh& mesh, Scene& scene) override;
	void setColor(uint32_t deviceIndex, const glm::vec3& color);
	void render() override;

	bool shouldClose() override;
	uint32_t getDeviceCount() const { return deviceCount; }

	void printMeasurements();

private:
	Strategy strategy;
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	Scene* scene;

	uint32_t deviceCount;
	std::vector<bp::Device> devices;
	std::vector<SecondaryRenderer> secondaryRenderers;
	std::vector<SFBenchSubpass> subpasses;

	bp::Texture renderColorAttachment;
	bp::Texture renderDepthAttachment;
	bp::RenderPass renderPass;

	std::vector<bp::Texture> compositingColorSources;
	std::vector<bp::Texture> compositingDepthSources;

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

	//Time measure
	std::mutex measureMut;
	std::unordered_map<std::string, double> measureAccumulators;
	unsigned measureFrameCount{0};
};


#endif
