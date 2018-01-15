#ifndef VMGPU_BORDERLESSSUBRENDERER_H
#define VMGPU_BORDERLESSSUBRENDERER_H

#include <bpView/Window.h>
#include <bp/Swapchain.h>
#include <bp/DepthAttachment.h>
#include <bp/RenderPass.h>
#include <bp/Semaphore.h>

class SortFirstBorderlessRenderer;

class BorderlessSubRenderer
{
	friend class SortFirstBorderlessRenderer;
public:
	BorderlessSubRenderer() :
		subpass{nullptr} {}

	void init(VkInstance instance, VkPhysicalDevice physicalDevice, bp::Subpass& subpass,
		  const VkRect2D& area);
	void render();
	void present();

private:
	bp::Window window;
	bp::Device device;
	bp::Swapchain swapchain;
	bp::DepthAttachment depthAttachment;
	bp::Subpass* subpass;
	bp::RenderPass renderPass;
	bp::Semaphore renderCompleteSem;
	bp::CommandPool cmdPool;
	VkCommandBuffer cmdBuffer;
};


#endif
