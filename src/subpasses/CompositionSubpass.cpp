#include "CompositionSubpass.h"
#include <bp/RenderPass.h>
#include <bp/Util.h>

using namespace bp;
using namespace std;

CompositionSubpass::~CompositionSubpass()
{
	if (sampler != VK_NULL_HANDLE) vkDestroySampler(*device, sampler, nullptr);
}

void CompositionSubpass::init(NotNull<RenderPass> renderPass)
{
	CompositionSubpass::renderPass = renderPass;

	initShaders();
	initDescriptorSetLayout();
	initPipelineLayout();
	initPipeline();
	initSampler();
	initTextures();
	initialized = true;
}

void CompositionSubpass::render(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	const VkRect2D& area = renderPass->getRenderArea();
	VkViewport viewport = {(float) area.offset.x, (float) area.offset.y,
			       (float) area.extent.width, (float) area.extent.height, 0.f, 1.f};
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &area);

	for (unsigned i = 0; i < textureCount; i++)
	{
		VkDescriptorSet set = descriptorSets[i];
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
					0, 1, &set, 0, nullptr);
		VkRect2D& current = areas[i];
		PushConstant push = {
			2.f * static_cast<float>(current.offset.x) /
			static_cast<float>(area.extent.width) - 1.f,
			2.f * static_cast<float>(current.offset.y) /
			static_cast<float>(area.extent.height) - 1.f,
			2.f * static_cast<float>(current.extent.width) /
			static_cast<float>(area.extent.width),
			2.f * static_cast<float>(current.extent.height) /
			static_cast<float>(area.extent.height)
		};
		vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
				   sizeof(PushConstant), &push);
		vkCmdDraw(cmdBuffer, 4, 1, 0, 0);
	}
}

unsigned CompositionSubpass::addTexture(const VkRect2D& area, NotNull<Texture> texture,
					Texture* depthTexture)
{
	if (initialized) throw runtime_error("Texture must be added before initialization.");
	textures.push_back(texture.get());
	if (depthTexture != nullptr) depthTextures.push_back(depthTexture);
	else if (depthTestEnabled) throw runtime_error("No depth texture provided.");
	areas.push_back(area);
	textureCount++;
}

void CompositionSubpass::resizeTextureResources(unsigned index, const VkRect2D& newArea)
{
	areas[index] = newArea;
	if (initialized)
	{
		descriptors[index].resetDescriptorInfos();
		descriptors[index].addDescriptorInfo({sampler, textures[index]->getImageView(),
						      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
		descriptorSets[index].bind(&descriptors[index]);
		if (depthTestEnabled)
		{
			depthDescriptors[index].resetDescriptorInfos();
			depthDescriptors[index].addDescriptorInfo(
				{sampler, depthTextures[index]->getImageView(),
				 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
			descriptorSets[index].bind(&depthDescriptors[index]);
		}
		descriptorSets[index].update();
	}
}

void CompositionSubpass::initShaders()
{
	auto vertexShaderCode = readBinaryFile("spv/compositeQuad.vert.spv");
	vertexShader.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));

	vector<char> fragmentShaderCode;
	if (depthTestEnabled)
		fragmentShaderCode = readBinaryFile("spv/directTextureDepthTest.frag.spv");
	else
		fragmentShaderCode = readBinaryFile("spv/directTexture.frag.spv");
	fragmentShader.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(fragmentShaderCode.size()),
			    reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));
}

void CompositionSubpass::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	if (depthTestEnabled)
	{
		descriptorSetLayout.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	}
	descriptorSetLayout.init(device);
}

void CompositionSubpass::initPipelineLayout()
{
	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant)});
	pipelineLayout.init(device);
}

void CompositionSubpass::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipeline.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
	pipeline.init(device, renderPass, pipelineLayout);
}

void CompositionSubpass::initSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkResult result = vkCreateSampler(*device, &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create sampler.");
}

void CompositionSubpass::initTextureResources(const VkRect2D& area)
{
	auto index = descriptors.size();
	descriptors.emplace_back();
	if (depthTestEnabled) depthDescriptors.emplace_back();
	descriptorSets.emplace_back();
	descriptorSets[index].init(device, &descriptorPool, &descriptorSetLayout);

	descriptors[index].setType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	descriptors[index].addDescriptorInfo({sampler, textures[index]->getImageView(),
					      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
	descriptorSets[index].bind(&descriptors[index]);
	if (depthTestEnabled)
	{
		depthDescriptors[index].setType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		depthDescriptors[index].setBinding(1);
		depthDescriptors[index].addDescriptorInfo(
			{sampler, depthTextures[index]->getImageView(),
			 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
		descriptorSets[index].bind(&depthDescriptors[index]);
	}
	descriptorSets[index].update();
}

void CompositionSubpass::initTextures()
{
	descriptorPool.init(device,
			    {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount * 2}},
			    textureCount);
	descriptorSets.reserve(textureCount);
	descriptors.reserve(textureCount);
	depthDescriptors.reserve(textureCount);
	for (const VkRect2D& area : areas) initTextureResources(area);
}