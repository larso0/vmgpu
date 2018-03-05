#ifndef VMGPU_MESHSUBPASS_H
#define VMGPU_MESHSUBPASS_H

#include <bp/Subpass.h>
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/Buffer.h>
#include <bpScene/Camera.h>
#include <bpScene/Mesh.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <bp/BufferDescriptor.h>

class MeshSubpass : public bp::Subpass
{
public:
	MeshSubpass() :
		initialized{false},
		renderPass{nullptr},
		mesh{nullptr},
		offset{0}, count{0},
		meshNode{nullptr},
		camera{nullptr},
		color{1.f, 0.f, 0.f} {}

	void setScene(bpScene::Mesh& mesh, uint32_t offset, uint32_t count,
		      bpScene::Node& meshNode, bpScene::Camera& camera)
	{
		MeshSubpass::mesh = &mesh;
		MeshSubpass::offset = offset;
		MeshSubpass::count = count;
		MeshSubpass::meshNode = &meshNode;
		MeshSubpass::camera = &camera;
	}

	void setClipTransform(float x, float y, float w, float h);
	void setColor(const glm::vec3& color);
	void init(bp::RenderPass& renderPass) override;
	void render(VkCommandBuffer cmdBuffer) override;

private:
	bool initialized;
	bp::RenderPass* renderPass;
	bpScene::Mesh* mesh;
	uint32_t offset, count;

	bp::Shader vertexShader, geometryShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	bp::Buffer vertexBuffer, indexBuffer, uniformBuffer;

	bp::DescriptorPool descriptorPool;
	bp::DescriptorSet descriptorSet;
	bp::BufferDescriptor uniformBufferDescriptor;

	struct PushConstant
	{
		glm::mat4 mvpMatrix;
		glm::mat4 normalMatrix;
	};

	bpScene::Node* meshNode;
	bpScene::Camera* camera;
	glm::mat4 clipTransform;
	glm::vec3 color;

	void initShaders();
	void initDescriptorSetLayout();
	void initPipelineLayout();
	void initPipeline();
	void initBuffers();
	void initDescriptors();
};


#endif
