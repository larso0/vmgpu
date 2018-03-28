#ifndef VMGPU_VMGPU_H
#define VMGPU_VMGPU_H

#include <bpQt/Window.h>

class Vmgpu : public bpQt::Window
{
public:
	Vmgpu(QVulkanInstance& instance) :
		bpQt::Window{instance}
	{
		setVSync(true);
		swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	}

private:
	void initRenderResources(uint32_t width, uint32_t height) override {}

	void resizeRenderResources(uint32_t width, uint32_t height) override
	{
		Window::resizeRenderResources(width, height);
	}

	void specifyDeviceRequirements(bp::DeviceRequirements& requirements) override {}
	void render(VkCommandBuffer cmdBuffer) override {}
	void update(double frameDeltaTime) override {}
};


#endif
