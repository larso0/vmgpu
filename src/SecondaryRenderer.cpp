#include "SecondaryRenderer.h"
#include <bp/Util.h>
#include <bp/Util.h>
#include <stdexcept>
#include <future>

using namespace bp;
using namespace std;

SecondaryRenderer::~SecondaryRenderer()
{
	if (renderDevice != nullptr)
	{
		colorAttachment.getImage().unmap(false);
		if (strategy == Strategy::SORT_LAST)
			depthAttachment.getImage().unmap(false);
	}
}

void SecondaryRenderer::init(Strategy strategy, Device& renderDevice, uint32_t width,
			     uint32_t height, Subpass& subpass)
{
	SecondaryRenderer::strategy = strategy;
	SecondaryRenderer::renderDevice = &renderDevice;
	SecondaryRenderer::subpass = &subpass;

	colorAttachment.setClearEnabled(true);
	colorAttachment.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	colorAttachment.init(renderDevice, VK_FORMAT_R8G8B8A8_UNORM,
			     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(renderDevice, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);

	subpass.addColorAttachment(colorAttachment);
	subpass.setDepthAttachment(depthAttachment);

	renderPass.addSubpassGraph(subpass);
	renderPass.init(width, height);
	renderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(renderDevice.getGraphicsQueue());

	copyDoneEvent.init(renderDevice);

	renderCmdBuffer = cmdPool.allocateCommandBuffer();
	queue = &renderDevice.getGraphicsQueue();

	colorSrc = colorAttachment.getImage().map(0, VK_WHOLE_SIZE);
	if (strategy == Strategy::SORT_LAST)
		depthSrc = depthAttachment.getImage().map(0, VK_WHOLE_SIZE);

	//Render the first frame
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCmdBuffer, &beginInfo);
	renderPass.render(renderCmdBuffer);
	colorAttachment.getImage().updateStagingBuffer(renderCmdBuffer);
	depthAttachment.getImage().updateStagingBuffer(renderCmdBuffer);
	vkEndCommandBuffer(renderCmdBuffer);

	queue->submit({}, {renderCmdBuffer}, {});
	queue->waitIdle();
}

void SecondaryRenderer::resize(uint32_t width, uint32_t height)
{
	colorAttachment.getImage().unmap(false);
	if (strategy == Strategy::SORT_LAST)
		depthAttachment.getImage().unmap(false);
	colorAttachment.resize(width, height);
	depthAttachment.resize(width, height);
	colorSrc = colorAttachment.getImage().map(0, VK_WHOLE_SIZE);
	if (strategy == Strategy::SORT_LAST)
		depthSrc = depthAttachment.getImage().map(0, VK_WHOLE_SIZE);

	renderPass.resize(width, height);
	renderPass.setRenderArea({{}, {width, height}});
}

void SecondaryRenderer::render()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCmdBuffer, &beginInfo);
	renderPass.render(renderCmdBuffer);

	VkEvent event = copyDoneEvent;
	vkCmdWaitEvents(renderCmdBuffer, 1, &event, VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);

	colorAttachment.getImage().updateStagingBuffer(renderCmdBuffer);
	depthAttachment.getImage().updateStagingBuffer(renderCmdBuffer);

	vkEndCommandBuffer(renderCmdBuffer);

	queue->submit({}, {renderCmdBuffer}, {});
	queue->waitIdle();
}

void SecondaryRenderer::copy(void* colorDst, void* depthDst)
{
	auto colorCopyFuture = async(launch::async, [&, this]{
		size_t colorSize = colorAttachment.getWidth() * colorAttachment.getHeight() * 4;
		parallelCopy(colorDst, colorSrc, colorSize);
	});

	if (strategy == Strategy::SORT_LAST)
	{
		size_t depthSize = depthAttachment.getWidth() * depthAttachment.getHeight() * 2;
		parallelCopy(depthDst, depthSrc, depthSize);
	}

	colorCopyFuture.wait();

	vkSetEvent(*renderDevice, copyDoneEvent);
}

void SecondaryRenderer::resetCopyDoneEvent()
{
	vkResetEvent(*renderDevice, copyDoneEvent);
}