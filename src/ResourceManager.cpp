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

void ResourceManager::addEntity(unsigned modelIndex, Node& node)
{
	unsigned drawableId = drawables.createResource();
	drawables[drawableId].init(pipeline, models[modelIndex]);
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, node, *camera);
	bpUtil::connect(drawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(drawables[drawableId]);
}

void ResourceManager::setClipTransform(const glm::mat4& transform)
{
	for (auto& pc : pushConstants) pc.setClipTransform(transform);
}

void ResourceManager::updatePushConstants()
{
	for (auto& pc : pushConstants) pc.update();
}
