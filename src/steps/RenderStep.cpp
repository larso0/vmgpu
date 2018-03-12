#include "RenderStep.h"
#include <bp/Util.h>
#include <bpScene/Vertex.h>

using namespace bp;
using namespace bpScene;

void RenderFramebuffer::init(RenderPass& renderPass, uint32_t width, uint32_t height,
			     const AttachmentSlot& colorSlot,
			     const AttachmentSlot& depthSlot)
{
	Device& device = renderPass.getDevice();
	colorAttachment.init(device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
							       | VK_IMAGE_USAGE_SAMPLED_BIT,
			     width, height);
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			     | VK_IMAGE_USAGE_SAMPLED_BIT, width, height);

	setAttachment(colorSlot, colorAttachment);
	setAttachment(depthSlot, depthAttachment);

	Framebuffer::init(renderPass, width, height);
}

void RenderStep::init(Device& device, unsigned outputCount, uint32_t width, uint32_t height)
{
	RenderStep::device = &device;
	RenderStep::width = width;
	RenderStep::height = height;
	queue = &device.getGraphicsQueue();
	cmdPool.init(*queue);
	cmdBuffer = cmdPool.allocateCommandBuffer();

	initShaders();
	initDescriptorSetLayout();
	initPipelineLayout();
	initPipeline();
	initBuffers();
	initDescriptors();

	colorAttachmentSlot.init(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
				 VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	depthAttachmentSlot.init(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT,
				 VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	initRenderPass();

	PipelineStep<Scene, RenderFramebuffer>::init(outputCount);
}

void RenderStep::resize(uint32_t width, uint32_t height)
{
	for (unsigned i = 0; i < getOutputCount(); i++)
		getOutput(i).resize(width, height);
}

void RenderStep::prepare(RenderFramebuffer& output)
{
	output.init(renderPass, width, height, colorAttachmentSlot, depthAttachmentSlot);
}

void RenderStep::process(Scene& input, RenderFramebuffer& output, unsigned outputIndx)
{
	//TODO check scene

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	renderPass.render(output, cmdBuffer);
	vkEndCommandBuffer(cmdBuffer);

	queue->submit({}, {cmdBuffer}, {});
	queue->waitIdle();
}

void RenderStep::initShaders()
{
	auto vertexShaderCode = readBinaryFile("spv/basic.vert.spv");
	vertexShader.init(*device, VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));
	auto fragmentShaderCode = readBinaryFile("spv/basic.frag.spv");
	fragmentShader.init(*device, VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(fragmentShaderCode.size()),
			    reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));
}

void RenderStep::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					      VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(*device);
}

void RenderStep::initPipelineLayout()
{
	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant)});
	pipelineLayout.init(*device);
}

void RenderStep::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.addVertexBindingDescription({0, Vertex::STRIDE, VK_VERTEX_INPUT_RATE_VERTEX});
	pipeline.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
						Vertex::POSITION_OFFSET});
	pipeline.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
						Vertex::NORMAL_OFFSET});
	pipeline.init(*device, renderPass, pipelineLayout);
}

void RenderStep::initBuffers()
{
	uniformBuffer.init(*device, sizeof(glm::vec3), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			   VMA_MEMORY_USAGE_GPU_ONLY);

	glm::vec3 color{1.f, 0.f, 0.f};
	uniformBuffer.transfer(0, sizeof(glm::vec3), &color);
}

void RenderStep::initDescriptors()
{
	descriptorPool.init(*device, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}}, 1);
	descriptorSet.init(*device, descriptorPool, descriptorSetLayout);
	uniformBufferDescriptor.setType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	uniformBufferDescriptor.addDescriptorInfo({uniformBuffer, 0, sizeof(glm::vec3)});
	descriptorSet.bind(uniformBufferDescriptor);
	descriptorSet.update();
}

void RenderStep::initRenderPass()
{
	subpass.addColorAttachment(colorAttachmentSlot);
	subpass.setDepthAttachment(depthAttachmentSlot);

	renderPass.addSubpassGraph(subpass);
	renderPass.setRenderArea({{0, 0}, {width, height}});
	renderPass.init(*device);
}