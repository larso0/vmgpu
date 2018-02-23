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
		for (auto b : colorStagingBuffers) delete b;
		if (strategy == Strategy::SORT_LAST)
			for (auto b : depthStagingBuffers) delete b;
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

	for (unsigned i = 0; i < 2; i++)
	{
		colorStagingBuffers[i] =
			new Buffer(renderDevice, colorAttachment.getImage().getMemorySize(),
				   VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if (strategy == Strategy::SORT_LAST)
		{
			depthStagingBuffers[i] =
				new Buffer(renderDevice,depthAttachment.getImage().getMemorySize(),
					   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY);
		}
	}

	subpass.addColorAttachment(colorAttachment);
	subpass.setDepthAttachment(depthAttachment);

	renderPass.addSubpassGraph(subpass);
	renderPass.init(width, height);
	renderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(renderDevice.getGraphicsQueue());

	cmdBuffer = cmdPool.allocateCommandBuffer();
	queue = &renderDevice.getGraphicsQueue();

	//Render the first frame
	render();
}

void SecondaryRenderer::resize(uint32_t width, uint32_t height)
{
	colorAttachment.resize(width, height);
	depthAttachment.resize(width, height);

	for (unsigned i = 0; i < 2; i++)
	{
		delete colorStagingBuffers[i];
		colorStagingBuffers[i] =
			new Buffer(*renderDevice, colorAttachment.getImage().getMemorySize(),
				   VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if (strategy == Strategy::SORT_LAST)
		{
			delete depthStagingBuffers[i];
			depthStagingBuffers[i] =
				new Buffer(*renderDevice,depthAttachment.getImage().getMemorySize(),
					   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY);
		}
	}

	renderPass.resize(width, height);
	renderPass.setRenderArea({{}, {width, height}});

	render();
}

void SecondaryRenderer::selectStagingBuffers()
{
	unsigned tmp = currentStagingBufferIndex;
	currentStagingBufferIndex = previousStagingBufferIndex;
	previousStagingBufferIndex = tmp;
}

void SecondaryRenderer::render()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	renderPass.render(cmdBuffer);
	colorStagingBuffers[currentStagingBufferIndex]->transfer(colorAttachment.getImage(),
								  cmdBuffer);
	if (strategy == Strategy::SORT_LAST)
		depthStagingBuffers[currentStagingBufferIndex]->transfer(
			depthAttachment.getImage(), cmdBuffer);
	vkEndCommandBuffer(cmdBuffer);

	queue->submit({}, {cmdBuffer}, {});
	queue->waitIdle();
}

void SecondaryRenderer::copy(void* colorDst, void* depthDst)
{
	auto colorCopyFuture = async(launch::async, [&, this]{
		size_t colorSize = colorAttachment.getWidth() * colorAttachment.getHeight() * 4;
		parallelCopy(colorDst, colorStagingBuffers[previousStagingBufferIndex]->map(),
			     colorSize);
	});

	if (strategy == Strategy::SORT_LAST)
	{
		size_t depthSize = depthAttachment.getWidth() * depthAttachment.getHeight() * 2;
		parallelCopy(depthDst, depthStagingBuffers[previousStagingBufferIndex]->map(),
			     depthSize);
	}

	colorCopyFuture.wait();
}