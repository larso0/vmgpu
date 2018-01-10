#ifndef VMGPU_COMPOSITIONSUBPASS_H
#define VMGPU_COMPOSITIONSUBPASS_H

#include <bp/Subpass.h>
#include <bp/Texture.h>
#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <bp/ImageDescriptor.h>
#include <vector>

class CompositionSubpass : public bp::Subpass
{
public:
	CompositionSubpass() :
		renderPass{nullptr},
		initialized{false},
		depthTestEnabled{false},
		sampler{VK_NULL_HANDLE},
		textureCount{0} {}
	~CompositionSubpass();

	void setDepthTestEnabled(bool enabled) { depthTestEnabled = enabled; }
	void init(bp::RenderPass& renderPass) override;
	void render(VkCommandBuffer cmdBuffer) override;

	unsigned addTexture(const VkRect2D& area, bp::Texture& texture);
	unsigned addTexture(const VkRect2D& area, bp::Texture& texture, bp::Texture& depthTexture);
	void resizeTextureResources(unsigned index, const VkRect2D& newArea);

	unsigned getTextureCount() const { return textureCount; }

private:
	struct PushConstant
	{
		float x, y; //Offset
		float w, h; //Extent
	};

	bp::RenderPass* renderPass;
	bool initialized;
	bool depthTestEnabled;
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	VkSampler sampler;

	unsigned textureCount;
	std::vector<bp::Texture*> textures;
	std::vector<bp::Texture*> depthTextures;
	std::vector<VkRect2D> areas;

	bp::DescriptorPool descriptorPool;
	std::vector<bp::DescriptorSet> descriptorSets;
	std::vector<bp::ImageDescriptor> descriptors;
	std::vector<bp::ImageDescriptor> depthDescriptors;

	void initShaders();
	void initDescriptorSetLayout();
	void initPipelineLayout();
	void initPipeline();
	void initSampler();
	void initDescriptors();
};


#endif
