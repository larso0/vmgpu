#include "SortLastSubRenderer.h"
#include <bp/Util.h>
#include "ParallelCopy.h"

using namespace bp;

SortLastSubRenderer::~SortLastSubRenderer()
{
	if (targetColorTexture != &colorAttachment) delete targetColorTexture;
}

void SortLastSubRenderer::init(NotNull<Device> renderDevice, NotNull<Device> targetDevice,
				uint32_t width, uint32_t height, NotNull<Subpass> subpass)
{
	SortLastSubRenderer::renderDevice = renderDevice;
	SortLastSubRenderer::targetDevice = targetDevice;
	SortLastSubRenderer::subpass = subpass;

	FlagSet<Texture::UsageFlags> colorUserFlags;
	colorUserFlags << Texture::UsageFlags::COLOR_ATTACHMENT;
	if (targetDevice == renderDevice)
		colorUserFlags << Texture::UsageFlags::SHADER_READABLE;

	colorAttachment.setUsageFlags(colorUserFlags);
	colorAttachment.setClearEnabled(true);
	colorAttachment.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	colorAttachment.init(renderDevice, VK_FORMAT_R8G8B8A8_UNORM, width, height);
	depthAttachment.setClearEnabled(true);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(renderDevice, width, height);
	targetDepthTexture.setUsageFlags(FlagSet<Texture::UsageFlags>()
						 << Texture::UsageFlags::SHADER_READABLE);
	targetDepthTexture.init(targetDevice, VK_FORMAT_D16_UNORM, width, height);

	if (targetDevice == renderDevice)
	{
		targetColorTexture = &colorAttachment;
	} else
	{
		targetColorTexture = new Texture(targetDevice, VK_FORMAT_R8G8B8A8_UNORM, width,
						 height, FlagSet<Texture::UsageFlags>()
							 << Texture::UsageFlags::SHADER_READABLE);
	}

	subpass->addColorAttachment(&colorAttachment);
	subpass->setDepthAttachment(&depthAttachment);

	renderPass.addSubpassGraph(subpass);
	renderPass.init(width, height);
	renderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(renderDevice->getGraphicsQueue());
	renderCmdBuffer = cmdPool.allocateCommandBuffer();
}

void SortLastSubRenderer::resize(uint32_t width, uint32_t height)
{
	colorAttachment.resize(width, height);
	depthAttachment.resize(width, height);
	targetDepthTexture.resize(width, height);
	if (targetDevice != renderDevice)
		targetColorTexture->resize(width, height);
	renderPass.resize(width, height);
	renderPass.setRenderArea({{}, {width, height}});
}

void SortLastSubRenderer::render()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCmdBuffer, &beginInfo);
	renderPass.render(renderCmdBuffer);

	depthAttachment.getImage()->updateStagingBuffer(renderCmdBuffer);
	if (targetDevice == renderDevice)
	{
		targetDepthTexture.getImage()->transfer(*depthAttachment.getImage(),
							renderCmdBuffer);
		targetColorTexture->transitionShaderReadable(renderCmdBuffer);
		targetDepthTexture.transitionShaderReadable(renderCmdBuffer);
	} else
	{
		colorAttachment.getImage()->updateStagingBuffer(renderCmdBuffer);
	}

	vkEndCommandBuffer(renderCmdBuffer);

	cmdPool.submit({}, {renderCmdBuffer}, {});
	cmdPool.waitQueueIdle();
}

void SortLastSubRenderer::copyToTarget()
{
	if (targetDevice != renderDevice)
	{
		const void* colorSrc = colorAttachment.getImage()->map(0, VK_WHOLE_SIZE);
		const void* depthSrc = depthAttachment.getImage()->map(0, VK_WHOLE_SIZE);
		void* colorDst = targetColorTexture->getImage()->map(0, VK_WHOLE_SIZE);
		void* depthDst = targetDepthTexture.getImage()->map(0, VK_WHOLE_SIZE);
		size_t colorSize = colorAttachment.getWidth() * colorAttachment.getHeight() * 4;
		size_t depthSize = depthAttachment.getWidth() * depthAttachment.getHeight() * 2;

		parallelCopy({{colorSrc, colorDst, colorSize},
			      {depthSrc, depthDst, depthSize}});

		colorAttachment.getImage()->unmap(false);
		depthAttachment.getImage()->unmap(false);
		targetColorTexture->getImage()->unmap();
		targetDepthTexture.getImage()->unmap();
	}
}