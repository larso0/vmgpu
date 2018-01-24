#ifndef VMGPU_SECONDARYRENDERER_H
#define VMGPU_SECONDARYRENDERER_H

#include <bp/Device.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/Event.h>

class SecondaryRenderer
{
public:
	enum class Strategy
	{
		SORT_FIRST,
		SORT_LAST
	};

	SecondaryRenderer() :
		strategy{Strategy::SORT_LAST},
		renderDevice{nullptr},
		subpass{nullptr},
		renderCmdBuffer{VK_NULL_HANDLE},
		queue{nullptr},
		colorSrc{nullptr},
		depthSrc{nullptr} {}
	~SecondaryRenderer();

	void init(Strategy strategy, bp::Device& renderDevice, uint32_t width, uint32_t height,
		  bp::Subpass& subpass);
	void resize(uint32_t width, uint32_t height);
	void render();
	void copy(void* colorDst, void* depthDst = nullptr);
	void resetCopyDoneEvent();

private:
	Strategy strategy;
	bp::Device* renderDevice;
	bp::Texture colorAttachment;
	bp::Texture depthAttachment;
	bp::RenderPass renderPass;
	bp::Subpass* subpass;
	bp::CommandPool cmdPool;
	bp::Event copyDoneEvent;
	VkCommandBuffer renderCmdBuffer;
	bp::Queue* queue;
	void* colorSrc;
	void* depthSrc;
};


#endif
