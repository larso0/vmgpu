#ifndef VMGPU_SORTLASTSUBRENDERER_H
#define VMGPU_SORTLASTSUBRENDERER_H

#include <bp/Device.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>

class SubRenderer
{
public:
	enum class Strategy
	{
		SORT_FIRST,
		SORT_LAST
	};

	SubRenderer() :
		strategy{Strategy::SORT_LAST},
		renderDevice{nullptr},
		targetDevice{nullptr},
		targetColorTexture{nullptr},
		targetDepthTexture{nullptr},
		subpass{nullptr},
		renderCmdBuffer{VK_NULL_HANDLE},
		queue{nullptr} {}
	~SubRenderer();

	void init(Strategy strategy, bp::Device& renderDevice, bp::Device& targetDevice,
		  uint32_t width, uint32_t height, bp::Subpass& subpass);
	void resize(uint32_t width, uint32_t height);
	void render();
	void copyToTarget();
	void prepareComposition(VkCommandBuffer cmdBuffer);

	bp::Texture& getTargetColorTexture() { return *targetColorTexture; }
	bp::Texture& getTargetDepthTexture();

private:
	Strategy strategy;
	bp::Device* renderDevice;
	bp::Device* targetDevice;
	bp::Texture colorAttachment;
	bp::Texture depthAttachment;
	bp::Texture* targetColorTexture;
	bp::Texture* targetDepthTexture;
	bp::RenderPass renderPass;
	bp::Subpass* subpass;
	bp::CommandPool cmdPool;
	VkCommandBuffer renderCmdBuffer;
	bp::Queue* queue;
};


#endif
