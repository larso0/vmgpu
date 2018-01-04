#include "MeshSubpass.h"
#include <bp/Util.h>

using namespace bp;
using namespace glm;

void MeshSubpass::setClipTransform(float x, float y, float w, float h)
{
	clipTransform = translate(scale(mat4{}, {1.f / w, 1.f / h, 1.f}),
				  {((0.5f + x) / w) - 0.5f, ((0.5f + y) / h) - 0.5f, 0.f});
}

void MeshSubpass::init(bp::NotNull<bp::RenderPass> renderPass)
{
	MeshSubpass::renderPass = renderPass;
	initShaders();
	initPipelineLayout();
	initPipeline();
	initBuffers();
}

void MeshSubpass::render(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	const VkRect2D& area = renderPass->getRenderArea();
	VkViewport viewport = {(float) area.offset.x, (float) area.offset.y,
			       (float) area.extent.width, (float) area.extent.height, 0.f, 1.f};
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &area);

	PushConstant pushConstant = {
		.mvpMatrix = clipTransform * camera->getProjectionMatrix()
			     * camera->getViewMatrix() * meshNode->getWorldMatrix(),
		.normalMatrix = transpose(inverse(meshNode->getWorldMatrix()))
	};
	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
			   sizeof(PushConstant), &pushConstant);

	VkDeviceSize vertexBufferOffset = 0;
	VkDeviceSize indexBufferOffset = offset * sizeof(uint32_t);
	VkBuffer vertexBufferHandle = vertexBuffer;
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBufferHandle, &vertexBufferOffset);
	vkCmdBindIndexBuffer(cmdBuffer, indexBuffer, indexBufferOffset, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, count, 1, 0, 0, 0);
}

void MeshSubpass::initShaders()
{
	auto vertexShaderCode = readBinaryFile("spv/basic.vert");
	vertexShader.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));
	auto fragmentShaderCode = readBinaryFile("spv/basic.frag");
	vertexShader.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			  static_cast<uint32_t>(fragmentShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));
}

void MeshSubpass::initPipelineLayout()
{
	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant)});
	pipelineLayout.init(device);
}

void MeshSubpass::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.init(device, renderPass, pipelineLayout);
}

void MeshSubpass::initBuffers()
{
	vertexBuffer.init(device, mesh->getVertexDataSize(),
			  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer.transfer(0, VK_WHOLE_SIZE, mesh->getVertexDataPtr());

	indexBuffer.init(device, mesh->getIndexDataSize(),
			 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.transfer(0, VK_WHOLE_SIZE, mesh->getIndexDataPtr());
}