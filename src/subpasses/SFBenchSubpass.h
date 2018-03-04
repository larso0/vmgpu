#ifndef VMGPU_SFBENCHSUBPASS_H
#define VMGPU_SFBENCHSUBPASS_H

#include <bp/Subpass.h>
#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>

class SFBenchSubpass : public bp::Subpass
{
public:
	void setArea(float x, float y, float w, float h) { pushConstants = { x, y, w, h }; }
	void init(bp::RenderPass& renderPass) override;
	void render(VkCommandBuffer cmdBuffer) override;
private:
	bp::RenderPass* renderPass{nullptr};
	bp::Shader vertexShader, fragmentShader;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	struct PushConstants
	{
		float x, y, w, h;
	} pushConstants{0.f, 0.f, 1.f, 1.f};

	void initShaders();
	void initPipelineLayout();
	void initPipeline();
};


#endif
