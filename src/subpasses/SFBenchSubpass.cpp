#include "SFBenchSubpass.h"
#include <bp/Util.h>

using namespace bp;

void SFBenchSubpass::init(RenderPass& renderPass)
{
	SFBenchSubpass::renderPass = &renderPass;
	initShaders();
	initPipelineLayout();
	initPipeline();
}

void SFBenchSubpass::render(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	const VkRect2D& area = renderPass->getRenderArea();
	VkViewport viewport = {(float) area.offset.x, (float) area.offset.y,
			       (float) area.extent.width, (float) area.extent.height, 0.f, 1.f};
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &area);

	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
			   sizeof(PushConstants),
			   reinterpret_cast<const void*>(&pushConstants));
	vkCmdDraw(cmdBuffer, 4, 1, 0, 0);
}

void SFBenchSubpass::initShaders()
{
	auto vertexShaderCode = readBinaryFile("spv/sfbench.vert.spv");
	vertexShader.init(*device, VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));
	auto fragmentShaderCode = readBinaryFile("spv/sfbench.frag.spv");
	fragmentShader.init(*device, VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(fragmentShaderCode.size()),
			    reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));
}

void SFBenchSubpass::initPipelineLayout()
{
	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)});
	pipelineLayout.init(*device);
}

void SFBenchSubpass::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
	pipeline.init(*device, *renderPass, pipelineLayout);
}