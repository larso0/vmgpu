#ifndef VMGPU_RESOURCEMANAGER_H
#define VMGPU_RESOURCEMANAGER_H

#include "ResourceList.h"
#include <bp/Device.h>
#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bpScene/Camera.h>
#include <bpScene/DrawableSubpass.h>
#include <bpScene/MeshResources.h>
#include <bpScene/MeshDrawable.h>
#include <bpScene/PushConstantResource.h>

class ResourceManager
{
public:
	void init(bp::Device& device, bp::RenderPass& renderPass, bpScene::Camera& camera);
	unsigned addMesh(bpScene::Mesh& mesh);
	void addEntity(unsigned meshIndex, bpScene::Node& node);
	void updatePushConstants();

	bp::Subpass& getSubpass() { return subpass; }
private:
	bp::Device* device;
	bpScene::Camera* camera;

	bpScene::DrawableSubpass subpass;
	bp::Shader vertexShader, fragmentShader;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	ResourceList<bpScene::MeshResources> meshes;
	ResourceList<bpScene::MeshDrawable> drawables;
	ResourceList<bpScene::PushConstantResource> pushConstants;
};


#endif
