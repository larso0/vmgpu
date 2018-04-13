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

	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
					      VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					      VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(device);

	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
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

unsigned ResourceManager::addModel(const Model& model)
{
	unsigned id = models.createResource();
	models[id].init(*device, descriptorSetLayout, 0, 1, model);
	return id;
}

unsigned ResourceManager::addMesh(const bpScene::Mesh& mesh, uint32_t offset, uint32_t count)
{
	unsigned id = meshes.createResource();
	meshes[id].init(*device, mesh, offset, count);
	return id;
}

void ResourceManager::addModelInstance(unsigned modelIndex, Node& node)
{
	unsigned drawableId = modelDrawables.createResource();
	modelDrawables[drawableId].init(pipeline, models[modelIndex]);
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, node, *camera);
	bpUtil::connect(modelDrawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(modelDrawables[drawableId]);
}

void ResourceManager::addMeshInstance(unsigned meshId, bpScene::Node& node)
{
	auto& mesh = meshes[meshId];
	unsigned drawableId = meshDrawables.createResource();
	meshDrawables[drawableId].init(pipeline, mesh, mesh.getOffset(), mesh.getElementCount());
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, node, *camera);
	bpUtil::connect(meshDrawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(meshDrawables[drawableId]);
}

void ResourceManager::setClipTransform(const glm::mat4& transform)
{
	for (auto& pc : pushConstants) pc.setClipTransform(transform);
}

void ResourceManager::updatePushConstants()
{
	for (auto& pc : pushConstants) pc.update();
}
