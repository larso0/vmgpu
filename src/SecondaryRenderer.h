#ifndef VMGPU_SECONDARYRENDERER_H
#define VMGPU_SECONDARYRENDERER_H

#include <bp/Device.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/Event.h>
#include <bp/Buffer.h>
#include <array>

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
		cmdBuffer{VK_NULL_HANDLE},
		queue{nullptr},
		currentStagingBufferIndex{0}, previousStagingBufferIndex{1} {}
	~SecondaryRenderer();

	void init(Strategy strategy, bp::Device& renderDevice, uint32_t width, uint32_t height,
		  bp::Subpass& subpass);
	void resize(uint32_t width, uint32_t height);
	void selectStagingBuffers();
	void render();
	void copy(void* colorDst, void* depthDst = nullptr);

private:
	Strategy strategy;
	bp::Device* renderDevice;
	bp::Texture colorAttachment;
	bp::Texture depthAttachment;
	std::array<bp::Buffer*, 2> colorStagingBuffers;
	std::array<bp::Buffer*, 2> depthStagingBuffers;
	bp::RenderPass renderPass;
	bp::Subpass* subpass;
	bp::CommandPool cmdPool;
	VkCommandBuffer cmdBuffer;
	bp::Queue* queue;
	unsigned currentStagingBufferIndex, previousStagingBufferIndex;
};


#endif
