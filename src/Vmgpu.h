#ifndef VMGPU_VMGPU_H
#define VMGPU_VMGPU_H

#include "Options.h"
#include "Scene.h"
#include "CameraController.h"
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
		camera{&cameraNode},
		mouseButton{false},
		rotate{false}
	{
		setContinuousRendering(true);
		setVSync(true);
	}

private:
	Options& options;

	bpScene::Camera camera;
	bpScene::Node cameraNode;
	CameraController cameraController;
	Scene scene;
	QPoint previousMousePos;
	bool mouseButton;
	bool rotate;

	std::vector<std::shared_ptr<bp::Device>> devices;
	std::vector<std::shared_ptr<bp::Renderer>> renderers;
	std::shared_ptr<bp::Renderer> mainRenderer;
	bp::Texture depthAttachment;
	bp::Framebuffer framebuffer;

	void initRenderResources(uint32_t width, uint32_t height) override;
	void initSortFirst(uint32_t width, uint32_t height);
	void initSortLast(uint32_t width, uint32_t height);

	void resizeRenderResources(uint32_t width, uint32_t height) override;
	void specifyDeviceRequirements(bp::DeviceRequirements& requirements) override;
	void render(VkCommandBuffer cmdBuffer) override;
	void update(double frameDeltaTime) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
};


#endif
