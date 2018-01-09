#ifndef VMGPU_SORTLASTSUBRENDERER_H
#define VMGPU_SORTLASTSUBRENDERER_H

#include <bp/Device.h>
#include <bp/Pointer.h>
#include <bp/Texture.h>
#include <bp/DepthAttachment.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>

class SortLastSubRenderer
{
public:
	SortLastSubRenderer() :
		renderDevice{nullptr},
		targetDevice{nullptr},
		targetColorTexture{nullptr},
		subpass{nullptr},
		renderCmdBuffer{VK_NULL_HANDLE} {}
	~SortLastSubRenderer();

	void init(bp::NotNull<bp::Device> renderDevice, bp::NotNull<bp::Device> targetDevice,
		  uint32_t width, uint32_t height, bp::NotNull<bp::Subpass> subpass);
	void resize(uint32_t width, uint32_t height);
	void render();
	void copyToTarget();
	void targetUnmap();

	bp::Texture* getTargetDepthTexture() { return &targetDepthTexture; }
	bp::Texture* getTargetColorTexture() { return targetColorTexture; }

private:
	bp::Device* renderDevice;
	bp::Device* targetDevice;
	bp::Texture colorAttachment;
	bp::DepthAttachment depthAttachment;
	bp::Texture targetDepthTexture;
	bp::Texture* targetColorTexture;
	bp::RenderPass renderPass;
	bp::Subpass* subpass;
	bp::CommandPool cmdPool;
	VkCommandBuffer renderCmdBuffer;
};


#endif
