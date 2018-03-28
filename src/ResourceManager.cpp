#include "ResourceManager.h"
#include <bp/Util.h>

using namespace bp;
using namespace bpScene;

void ResourceManager::init(Device& device, RenderPass& renderPass, Camera& camera)
{
	ResourceManager::device = &device;
	ResourceManager::camera = &camera;
	
	auto vertexShaderCode = readBinaryFile("spv/basic.vert.spv");
	vertexShader.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));
	auto fragmentShaderCode = readBinaryFile("spv/basic.frag.spv");
	fragmentShader.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(fragmentShaderCode.size()),
			    reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));

	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0,
					     sizeof(PushConstantResource::Matrices)});
	pipelineLayout.init(device);

	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.addVertexBindingDescription({0, Vertex::STRIDE,
						      VK_VERTEX_INPUT_RATE_VERTEX});
	pipeline.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::POSITION_OFFSET});
	pipeline.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::NORMAL_OFFSET});
	pipeline.init(device, renderPass, pipelineLayout);
}

unsigned ResourceManager::addMesh(Mesh& mesh)
{
	return meshes.createResource(*device, mesh);
}

void ResourceManager::addEntity(unsigned meshIndex, Node& node)
{
	unsigned drawableId = drawables.createResource(pipeline, meshes[meshIndex], 0,
						       meshes[meshIndex].getElementCount());
	unsigned pushId = pushConstants.createResource(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
						       node, *camera);
	bpUtil::connect(drawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(drawables[drawableId]);
}

void ResourceManager::updatePushConstants()
{
	for (auto& pc : pushConstants) pc.update();
}