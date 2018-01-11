#include "SubRenderer.h"
#include <bp/Util.h>
#include <bp/Util.h>
#include <stdexcept>
#include <future>

using namespace bp;
using namespace std;

SubRenderer::~SubRenderer()
{
	if (targetColorTexture != &colorAttachment) delete targetColorTexture;
	delete targetDepthTexture;
}

void SubRenderer::init(Strategy strategy, Device& renderDevice, Device& targetDevice,
		       uint32_t width, uint32_t height, Subpass& subpass)
{
	SubRenderer::strategy = strategy;
	SubRenderer::renderDevice = &renderDevice;
	SubRenderer::targetDevice = &targetDevice;
	SubRenderer::subpass = &subpass;

	FlagSet<Texture::UsageFlags> colorUserFlags;
	colorUserFlags << Texture::UsageFlags::COLOR_ATTACHMENT;
	if (&targetDevice == &renderDevice)
		colorUserFlags << Texture::UsageFlags::SHADER_READABLE;

	colorAttachment.setUsageFlags(colorUserFlags);
	colorAttachment.setClearEnabled(true);
	colorAttachment.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	colorAttachment.init(renderDevice, VK_FORMAT_R8G8B8A8_UNORM, width, height);
	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(renderDevice, width, height);

	if (strategy == Strategy::SORT_LAST)
	{
		targetDepthTexture = new Texture(targetDevice, VK_FORMAT_D16_UNORM, width, height,
						 FlagSet<Texture::UsageFlags>()
							 << Texture::UsageFlags::SHADER_READABLE);
	}

	if (&targetDevice == &renderDevice)
	{
		targetColorTexture = &colorAttachment;
	} else
	{
		targetColorTexture = new Texture(targetDevice, VK_FORMAT_R8G8B8A8_UNORM, width,
						 height, FlagSet<Texture::UsageFlags>()
							 << Texture::UsageFlags::SHADER_READABLE);
	}

	subpass.addColorAttachment(colorAttachment);
	subpass.setDepthAttachment(depthAttachment);

	renderPass.addSubpassGraph(subpass);
	renderPass.init(width, height);
	renderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(renderDevice.getGraphicsQueue());
	renderCmdBuffer = cmdPool.allocateCommandBuffer();
}

void SubRenderer::resize(uint32_t width, uint32_t height)
{
	colorAttachment.resize(width, height);
	depthAttachment.resize(width, height);
	if (strategy == Strategy::SORT_LAST)
		targetDepthTexture->resize(width, height);
	if (targetDevice != renderDevice)
		targetColorTexture->resize(width, height);
	renderPass.resize(width, height);
	renderPass.setRenderArea({{}, {width, height}});
}

void SubRenderer::render()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCmdBuffer, &beginInfo);
	renderPass.render(renderCmdBuffer);

	if (targetDevice == renderDevice)
	{
		if (strategy == Strategy::SORT_LAST)
		{
			targetDepthTexture->getImage().transfer(depthAttachment.getImage(),
								renderCmdBuffer);
			targetDepthTexture->transitionShaderReadable(renderCmdBuffer);
		}
		targetColorTexture->transitionShaderReadable(renderCmdBuffer);
	} else
	{
		depthAttachment.getImage().updateStagingBuffer(renderCmdBuffer);
		colorAttachment.getImage().updateStagingBuffer(renderCmdBuffer);
	}

	vkEndCommandBuffer(renderCmdBuffer);

	cmdPool.submit({}, {renderCmdBuffer}, {});
	cmdPool.waitQueueIdle();
}

void SubRenderer::copyToTarget()
{
	if (targetDevice != renderDevice)
	{
		auto colorCopy = async(launch::async, [this]{
			const void* colorSrc = colorAttachment.getImage().map(0, VK_WHOLE_SIZE);
			void* colorDst = targetColorTexture->getImage().map(0, VK_WHOLE_SIZE);
			size_t colorSize = colorAttachment.getWidth() * colorAttachment.getHeight()
					   * 4;
			parallelCopy(colorDst, colorSrc, colorSize);
			colorAttachment.getImage().unmap(false);
			targetColorTexture->getImage().unmap(false);
		});

		if (strategy == Strategy::SORT_LAST)
		{
			const void* depthSrc = depthAttachment.getImage().map(0, VK_WHOLE_SIZE);
			void* depthDst = targetDepthTexture->getImage().map(0, VK_WHOLE_SIZE);
			size_t depthSize = depthAttachment.getWidth() * depthAttachment.getHeight()
					   * 2;
			parallelCopy(depthDst, depthSrc, depthSize);
			depthAttachment.getImage().unmap(false);
			targetDepthTexture->getImage().unmap(false);
		}

		colorCopy.wait();
	}
}

void SubRenderer::prepareComposition(VkCommandBuffer cmdBuffer)
{
	targetColorTexture->getImage().flushStagingBuffer(cmdBuffer);
	targetColorTexture->getImage().transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_ACCESS_SHADER_READ_BIT,
						  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, cmdBuffer);
	if (strategy == Strategy::SORT_LAST)
	{
		targetDepthTexture->getImage().flushStagingBuffer(cmdBuffer);
		targetDepthTexture->getImage().transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
							  VK_ACCESS_SHADER_READ_BIT,
							  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							  cmdBuffer);
	}
}

bp::Texture& SubRenderer::getTargetDepthTexture()
{
	if (strategy == Strategy::SORT_FIRST)
		throw runtime_error("Sort first sub renderer does not have a target depth texture");
	return *targetDepthTexture;
}