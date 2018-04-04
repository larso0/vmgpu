#ifndef VMGPU_VMGPU_H
#define VMGPU_VMGPU_H

#include "Options.h"
#include <bp/Texture.h>
#include <bp/Framebuffer.h>
#include <bp/Renderer.h>
#include <bpQt/Window.h>
#include <bpScene/Mesh.h>
#include <bpScene/Camera.h>
#include <vector>
#include <memory>

class Vmgpu : public bpQt::Window
{
public:
	Vmgpu(QVulkanInstance& instance, Options& options) :
		bpQt::Window{instance},
		options{options},
		camera{&cameraNode}
	{
		setVSync(true);
	}

private:
	Options& options;
	bpScene::Mesh mesh;
	bpScene::Camera camera;
	bpScene::Node cameraNode;
	bpScene::Node objectNode;

	std::vector<std::shared_ptr<bp::Device>> devices;
	std::vector<std::shared_ptr<bp::Renderer>> renderers;
	std::shared_ptr<bp::Renderer> mainRenderer;
	bp::Texture depthAttachment;
	bp::Framebuffer framebuffer;

	void initRenderResources(uint32_t width, uint32_t height) override;
	void initSortFirst(uint32_t width, uint32_t height);
	void initSortLast(uint32_t width, uint32_t height);

	void resizeRenderResources(uint32_t width, uint32_t height) override
	{
		Window::resizeRenderResources(width, height);
	}

	void specifyDeviceRequirements(bp::DeviceRequirements& requirements) override;
	void render(VkCommandBuffer cmdBuffer) override {}
	void update(double frameDeltaTime) override {}
};


#endif
