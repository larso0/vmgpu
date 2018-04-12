#ifndef VMGPU_RESOURCEMANAGER_H
#define VMGPU_RESOURCEMANAGER_H

#include <bpScene/ResourceList.h>
#include <bp/Device.h>
#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bpScene/Camera.h>
#include <bpScene/DrawableSubpass.h>
#include <bpScene/ModelResources.h>
#include <bpScene/ModelDrawable.h>
#include <bpScene/PushConstantResource.h>

class ResourceManager
{
public:
	void init(bp::Device& device, bp::RenderPass& renderPass, bpScene::Camera& camera);
	unsigned addModel(const bpScene::Model& model);
	void addEntity(unsigned modelIndex, bpScene::Node& node);
	void setClipTransform(const glm::mat4& transform);
	void updatePushConstants();

	bp::Subpass& getSubpass() { return subpass; }
private:
	bp::Device* device;
	bpScene::Camera* camera;

	bpScene::DrawableSubpass subpass;
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	bpScene::ResourceList<bpScene::ModelResources> models;
	bpScene::ResourceList<bpScene::ModelDrawable> drawables;
	bpScene::ResourceList<bpScene::PushConstantResource> pushConstants;
};


#endif
