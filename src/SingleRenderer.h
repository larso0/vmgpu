#ifndef VMGPU_SINGLERENDERER_H
#define VMGPU_SINGLERENDERER_H

#include "Renderer.h"
#include <bpView/Window.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/Semaphore.h>
#include "subpasses/MeshSubpass.h"

class SingleRenderer : public Renderer
{
public:
	SingleRenderer() :
		instance{nullptr},
		mesh{nullptr},
		scene{nullptr},
		cmdBuffer{VK_NULL_HANDLE},
		queue{nullptr} {}

	void init(bp::Instance& instance, uint32_t width, uint32_t height,
		  bpScene::Mesh& mesh, Scene& scene) override;
	void render() override;

	bool shouldClose() override;

private:
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	Scene* scene;

	bpView::Window window;
	bp::Device device;
	bp::Swapchain swapchain;
	bp::Texture depthAttachment;
	MeshSubpass meshSubpass;
	bp::RenderPass renderPass;
	bp::CommandPool cmdPool;
	bp::Semaphore renderCompleteSem;
	VkCommandBuffer cmdBuffer;
	bp::Queue* queue;
};


#endif
