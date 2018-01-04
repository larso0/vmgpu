#ifndef VMGPU_MESHSUBPASS_H
#define VMGPU_MESHSUBPASS_H

#include <bp/Subpass.h>
#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/Buffer.h>
#include <bpScene/Camera.h>
#include <bpScene/Mesh.h>

class MeshSubpass : public bp::Subpass
{
public:
	MeshSubpass() :
		renderPass{nullptr},
		mesh{nullptr},
		offset{0}, count{0},
		meshNode{nullptr},
		camera{nullptr} {}

	void setScene(bp::NotNull<bpScene::Mesh> mesh, uint32_t offset, uint32_t count,
		      bp::NotNull<bpScene::Node> meshNode, bp::NotNull<bpScene::Camera> camera)
	{
		MeshSubpass::mesh = mesh;
		MeshSubpass::offset = offset;
		MeshSubpass::count = count;
		MeshSubpass::meshNode = meshNode;
		MeshSubpass::camera = camera;
	}

	void setClipTransform(float x, float y, float w, float h);
	void init(bp::NotNull<bp::RenderPass> renderPass) override;
	void render(VkCommandBuffer cmdBuffer) override;

private:
	bp::RenderPass* renderPass;
	bpScene::Mesh* mesh;
	uint32_t offset, count;

	bp::Shader vertexShader, fragmentShader;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	bp::Buffer vertexBuffer, indexBuffer;

	struct PushConstant
	{
		glm::mat4 mvpMatrix;
		glm::mat4 normalMatrix;
	};

	bpScene::Node* meshNode;
	bpScene::Camera* camera;
	glm::mat4 clipTransform;

	void initShaders();
	void initPipelineLayout();
	void initPipeline();
	void initBuffers();
};


#endif
