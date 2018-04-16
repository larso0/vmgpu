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
#include <bpScene/MeshDrawable.h>
#include <bpScene/PushConstantResource.h>

class ResourceManager
{
public:
	ResourceManager() :
		device{nullptr},
		camera{nullptr},
		scale{0.f} {}

	void init(bp::Device& device, bp::RenderPass& renderPass, bpScene::Camera& camera);
	unsigned addModel(const bpScene::Model& model);
	unsigned addMesh(const bpScene::Mesh& mesh, uint32_t offset, uint32_t count);
	unsigned addMesh(const bpScene::Mesh& mesh)
	{
		return addMesh(mesh, 0, mesh.getElementCount());
	}
	void addModelInstance(unsigned modelIndex, bpScene::Node& node);
	void addMeshInstance(unsigned meshId, bpScene::Node& node);
	void setClipTransform(const glm::mat4& transform);
	void updatePushConstants();

	bp::Subpass& getSubpass() { return subpass; }
private:
	bp::Device* device;
	bpScene::Camera* camera;

	bpScene::DrawableSubpass subpass;
	bp::Shader vertexBasic, vertexBasicUv, fragmentBasic, fragmentColored, fragmentTextured;
	bp::DescriptorSetLayout setLayoutColored, setLayoutTextured;
	bp::PipelineLayout pipelineLayoutBasic, pipelineLayoutColored, pipelineLayoutTextured;
	bp::GraphicsPipeline pipelineBasic, pipelineColored, pipelineTextured;

	bpScene::ResourceList<bpScene::ModelResources> models;
	std::vector<bool> modelIsTextured;
	bpScene::ResourceList<bpScene::ModelDrawable> modelDrawables;
	bpScene::ResourceList<bpScene::MeshResources> meshes;
	bpScene::ResourceList<bpScene::MeshDrawable> meshDrawables;
	bpScene::ResourceList<bpScene::PushConstantResource> pushConstants;

	float scale;
};


#endif
