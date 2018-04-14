#include "ResourceManager.h"
#include <bp/Util.h>
#include <glm/gtx/component_wise.hpp>

using namespace bp;
using namespace bpScene;

void ResourceManager::init(Device& device, RenderPass& renderPass, Camera& camera)
{
	ResourceManager::device = &device;
	ResourceManager::camera = &camera;

	//Load shader byte code
	auto shaderCode = readBinaryFile("spv/basic.vert.spv");
	vertexBasic.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			 static_cast<uint32_t>(shaderCode.size()),
			 reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = readBinaryFile("spv/basicUv.vert.spv");
	vertexBasicUv.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			   static_cast<uint32_t>(shaderCode.size()),
			   reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = readBinaryFile("spv/basic.frag.spv");
	fragmentBasic.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			   static_cast<uint32_t>(shaderCode.size()),
			   reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = readBinaryFile("spv/material.frag.spv");
	fragmentColored.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			     static_cast<uint32_t>(shaderCode.size()),
			     reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = readBinaryFile("spv/materialTextured.frag.spv");
	fragmentTextured.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			      static_cast<uint32_t>(shaderCode.size()),
			      reinterpret_cast<const uint32_t*>(shaderCode.data()));

	//Setup descriptor set layouts
	setLayoutColored.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					    VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutColored.init(device);

	setLayoutTextured.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					    VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutTextured.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
					      VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutTextured.init(device);

	//Setup pipeline layouts
	pipelineLayoutBasic.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0,
						  sizeof(PushConstantResource::Matrices)});
	pipelineLayoutBasic.init(device);

	pipelineLayoutColored.addDescriptorSetLayout(setLayoutColored);
	pipelineLayoutColored.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0,
						    sizeof(PushConstantResource::Matrices)});
	pipelineLayoutColored.init(device);

	pipelineLayoutTextured.addDescriptorSetLayout(setLayoutTextured);
	pipelineLayoutTextured.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0,
						     sizeof(PushConstantResource::Matrices)});
	pipelineLayoutTextured.init(device);
	
	//Setup pipelines
	pipelineBasic.addShaderStageInfo(vertexBasic.getPipelineShaderStageInfo());
	pipelineBasic.addShaderStageInfo(fragmentBasic.getPipelineShaderStageInfo());
	pipelineBasic.addVertexBindingDescription({0, Vertex::STRIDE,
						   VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineBasic.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
						     Vertex::POSITION_OFFSET});
	pipelineBasic.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
						     Vertex::NORMAL_OFFSET});
	pipelineBasic.init(device, renderPass, pipelineLayoutBasic);

	pipelineColored.addShaderStageInfo(vertexBasic.getPipelineShaderStageInfo());
	pipelineColored.addShaderStageInfo(fragmentColored.getPipelineShaderStageInfo());
	pipelineColored.addVertexBindingDescription({0, Vertex::STRIDE,
						     VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineColored.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
						       Vertex::POSITION_OFFSET});
	pipelineColored.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
						       Vertex::NORMAL_OFFSET});
	pipelineColored.init(device, renderPass, pipelineLayoutColored);

	pipelineTextured.addShaderStageInfo(vertexBasicUv.getPipelineShaderStageInfo());
	pipelineTextured.addShaderStageInfo(fragmentTextured.getPipelineShaderStageInfo());
	pipelineTextured.addVertexBindingDescription({0, Vertex::STRIDE,
						      VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineTextured.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::POSITION_OFFSET});
	pipelineTextured.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::NORMAL_OFFSET});
	pipelineTextured.addVertexAttributeDescription({2, 0, VK_FORMAT_R32G32_SFLOAT,
							Vertex::TEXTURE_COORDINATE_OFFSET});
	pipelineTextured.init(device, renderPass, pipelineLayoutTextured);
}

unsigned ResourceManager::addModel(const Model& model)
{
	unsigned id = models.createResource();
	DescriptorSetLayout* setLayout;
	modelIsTextured.push_back(model.getMaterial(0).isTextured());
	modelScale.push_back(1.f / glm::compMax(model.getSize()));
	if (modelIsTextured[id]) setLayout = &setLayoutTextured;
	else setLayout = &setLayoutColored;
	models[id].init(*device, *setLayout, 1, 0, model);
	return id;
}

unsigned ResourceManager::addMesh(const bpScene::Mesh& mesh, uint32_t offset, uint32_t count)
{
	unsigned id = meshes.createResource();
	meshes[id].init(*device, mesh, offset, count);
	meshScale.push_back(glm::compMax(mesh.getMaxVertex() - mesh.getMinVertex()));
	return id;
}

void ResourceManager::addModelInstance(unsigned modelIndex, Node& node)
{
	unsigned drawableId = modelDrawables.createResource();

	modelDrawables[drawableId].init(modelIsTextured[modelIndex] ? pipelineTextured
								    : pipelineColored,
					models[modelIndex]);
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(modelIsTextured[modelIndex] ? pipelineLayoutTextured
							       : pipelineLayoutColored,
				   VK_SHADER_STAGE_VERTEX_BIT, node, *camera);
	float scale = modelScale[modelIndex];
	pushConstants[pushId].setScaleTransform(glm::scale(glm::mat4{}, {scale, scale, scale}));
	bpUtil::connect(modelDrawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(modelDrawables[drawableId]);
}

void ResourceManager::addMeshInstance(unsigned meshId, bpScene::Node& node)
{
	auto& mesh = meshes[meshId];
	unsigned drawableId = meshDrawables.createResource();
	meshDrawables[drawableId].init(pipelineBasic, mesh, mesh.getOffset(),
				       mesh.getElementCount());
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(pipelineLayoutBasic, VK_SHADER_STAGE_VERTEX_BIT, node, *camera);
	float scale = meshScale[meshId];
	pushConstants[pushId].setScaleTransform(glm::scale(glm::mat4{}, {scale, scale, scale}));
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
