#ifndef VMGPU_RENDERSTEP_H
#define VMGPU_RENDERSTEP_H

#include "PipelineStep.h"
#include "../Scene.h"
#include <bp/Framebuffer.h>
#include <bp/Texture.h>
#include <bp/RenderPass.h>
#include <bp/CommandPool.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <bp/BufferDescriptor.h>
#include <bp/Buffer.h>
#include <bpScene/DrawableSubpass.h>
#include <bpScene/Math.h>

class RenderFramebuffer : public bp::Framebuffer
{
public:
	RenderFramebuffer() : bp::Framebuffer{} {}
	void init(bp::RenderPass& renderPass, uint32_t width, uint32_t height,
		  const bp::AttachmentSlot& colorSlot, const bp::AttachmentSlot& depthSlot);

	bp::Texture& getColorAttachment() { return colorAttachment; }
	bp::Texture& getDepthAttachment() { return depthAttachment; }
private:
	bp::Texture colorAttachment;
	bp::Texture depthAttachment;
};

class RenderStep : public PipelineStep<Scene, RenderFramebuffer>
{
public:
	RenderStep() :
		width{0}, height{0},
		device{nullptr},
		queue{nullptr},
		cmdBuffer{VK_NULL_HANDLE} {}
	RenderStep(bp::Device& device, unsigned outputCount, uint32_t width, uint32_t height) :
		RenderStep{}
	{
		init(device, outputCount, width, height);
	}

	void init(bp::Device& device, unsigned outputCount, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height);

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

private:
	uint32_t width, height;
	bp::Device* device;
	bp::Queue* queue;
	bp::CommandPool cmdPool;
	VkCommandBuffer cmdBuffer;

	bp::AttachmentSlot colorAttachmentSlot, depthAttachmentSlot;
	bpScene::DrawableSubpass subpass;
	bp::RenderPass renderPass;

	bp::Shader vertexShader, fragmentShader;
	bp::Buffer uniformBuffer;

	struct PushConstant
	{
		glm::mat4 mvpMatrix;
		glm::mat4 normalMatrix;
	};

	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	bp::DescriptorPool descriptorPool;
	bp::DescriptorSet descriptorSet;
	bp::BufferDescriptor uniformBufferDescriptor;

	void prepare(RenderFramebuffer& output) override;
	void process(Scene& input, RenderFramebuffer& output, unsigned outputIndx) override;

	void initShaders();
	void initDescriptorSetLayout();
	void initPipelineLayout();
	void initPipeline();
	void initBuffers();
	void initDescriptors();
	void initRenderPass();
};


#endif
