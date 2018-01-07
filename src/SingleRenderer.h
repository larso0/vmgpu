#ifndef VMGPU_SINGLERENDERER_H
#define VMGPU_SINGLERENDERER_H

#include "Renderer.h"
#include <bpView/Window.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/DepthAttachment.h>
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
		meshNode{&sceneRoot},
		cameraNode{&sceneRoot},
		camera{&cameraNode},
		cmdBuffer{VK_NULL_HANDLE} {}

	void init(bp::NotNull<bp::Instance> instance, uint32_t width, uint32_t height,
		  bp::NotNull<bpScene::Mesh> mesh) override;
	void render() override;

	bool shouldClose() override;

private:
	bp::Instance* instance;
	bpScene::Mesh* mesh;
	bpScene::Node sceneRoot, meshNode, cameraNode;
	bpScene::Camera camera;

	bp::Window window;
	bp::Device device;
	bp::Swapchain swapchain;
	bp::DepthAttachment depthAttachment;
	MeshSubpass meshSubpass;
	bp::RenderPass renderPass;
	bp::CommandPool cmdPool;
	bp::Semaphore renderCompleteSem;

	VkCommandBuffer cmdBuffer;
};


#endif
