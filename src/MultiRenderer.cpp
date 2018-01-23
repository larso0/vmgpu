#include "MultiRenderer.h"
#include <algorithm>
#include <future>

using namespace bp;
using namespace std;

void MultiRenderer::init(Instance& instance, uint32_t width, uint32_t height, bpScene::Mesh& mesh)
{
	MultiRenderer::instance = &instance;
	MultiRenderer::mesh = &mesh;

	devices.reserve(deviceCount);
	subRenderers.reserve(deviceCount);
	subpasses.reserve(deviceCount);

	//Setup scene
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	sceneRoot.update();
	camera.update();

	//Initialize window
	window.init(instance, width, height, "vmgpu");
	width = window.getWidth();
	height = window.getHeight();

	//Find device to create the swapchain for the window
	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.surface = window;
	requirements.extensions.push_back("VK_KHR_swapchain");
	devices.emplace_back(instance, requirements);
	queue = &devices[0].getGraphicsQueue();

	//Setup attachments and subpass for composition
	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.0});
	swapchain.init(devices[0], window, width, height, false);

	compositionSubpass.addColorAttachment(swapchain);

	if (strategy == Strategy::SORT_LAST)
	{
		depthAttachment.setClearEnabled(true);
		depthAttachment.setClearValue({1.f, 0.f});
		depthAttachment.init(devices[0], VK_FORMAT_D16_UNORM,
				     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);
		compositionSubpass.setDepthTestEnabled(true);
		compositionSubpass.setDepthAttachment(depthAttachment);
	}

	compositionRenderPass.addSubpassGraph(compositionSubpass);

	//Find the remaining devices
	requirements.surface = VK_NULL_HANDLE;
	requirements.extensions.clear();
	auto availableDevices = queryDevices(instance, requirements);

	for (uint32_t i = 1; i < deviceCount; i++)
	{
		VkPhysicalDevice selected = devices[0];
		for (VkPhysicalDevice device : availableDevices)
		{
			if (!isDeviceChosen(device))
			{
				selected = device;
				break;
			}
		}
		devices.emplace_back(selected, requirements);
	}

	auto renderAreas = calcululateSubRendererAreas(width, height);
	auto portions = calculateMeshPortions();

	//Setup sub renderers
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		const auto& area = renderAreas[i];

		subpasses.emplace_back();
		subpasses[i].setScene(mesh, portions[i].first, portions[i].second, meshNode,
				      camera);
		subpasses[i].setClipTransform(
			static_cast<float>(area.offset.x) / static_cast<float>(width),
			static_cast<float>(area.offset.y) / static_cast<float>(height),
			static_cast<float>(area.extent.width) / static_cast<float>(width),
			static_cast<float>(area.extent.height) / static_cast<float>(height)
		);

		subRenderers.emplace_back();
		subRenderers[i].init(strategy, devices[i], devices[0], renderAreas[i].extent.width,
				     renderAreas[i].extent.height, subpasses[i]);

		if (strategy == Strategy::SORT_LAST)
		{
			compositionSubpass.addTexture(renderAreas[i],
						      subRenderers[i].getTargetColorTexture(),
						      subRenderers[i].getTargetDepthTexture());
		} else
		{
			compositionSubpass.addTexture(renderAreas[i],
						      subRenderers[i].getTargetColorTexture());
		}
	}

	//Handle window resize events
	connect(window.resizeEvent, swapchain, &Swapchain::resize);

	//Resize resources when the swapchain is resized
	connect(swapchain.resizeEvent, [this](uint32_t w, uint32_t h){
		if (strategy == Strategy::SORT_LAST) depthAttachment.resize(w, h);
		compositionRenderPass.setRenderArea({{}, {w, h}});
		float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
		auto renderAreas = calcululateSubRendererAreas(w, h);
		for (auto i = 0; i < deviceCount; i++)
		{
			const auto& area = renderAreas[i];
			subpasses[i].setClipTransform(
				static_cast<float>(area.offset.x) / static_cast<float>(w),
				static_cast<float>(area.offset.y) / static_cast<float>(h),
				static_cast<float>(area.extent.width) / static_cast<float>(w),
				static_cast<float>(area.extent.height) / static_cast<float>(h)
			);
			subRenderers[i].resize(area.extent.width, area.extent.height);
			compositionSubpass.resizeTextureResources(i, area);
		}
	});

	//Initialize resources for compositing
	compositionRenderPass.init(width, height);
	compositionRenderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(devices[0].getGraphicsQueue());
	compositionPassCompleteSem.init(devices[0]);
	compositionCmdBuffer = cmdPool.allocateCommandBuffer();
}

void MultiRenderer::setColor(uint32_t deviceIndex, const glm::vec3& color)
{
	subpasses[deviceIndex].setColor(color);
}

void MultiRenderer::render()
{
	window.handleEvents();

	vector<future<void>> futures;

	//Render
	for (auto& r : subRenderers)
		futures.push_back(async(launch::async, [&r]{ r.render(); }));
	for (auto& f : futures) f.wait();

	//Copy to GPU responsible for compositing
	futures.clear();
	for (auto& r : subRenderers)
		futures.push_back(async(launch::async, [&r]{ r.copyToTarget(); }));
	for (auto& f : futures) f.wait();

	//Compositing
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(compositionCmdBuffer, &beginInfo);

	for (auto& r : subRenderers) r.prepareComposition(compositionCmdBuffer);

	compositionRenderPass.render(compositionCmdBuffer);
	vkEndCommandBuffer(compositionCmdBuffer);

	queue->submit({{swapchain.getImageAvailableSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		      {compositionCmdBuffer}, {compositionPassCompleteSem});
	queue->waitIdle();

	swapchain.present(compositionPassCompleteSem);
}

void MultiRenderer::update(float delta)
{
	meshNode.rotate(delta, {0.f, 1.f, 0.f});
	meshNode.update();
}

bool MultiRenderer::shouldClose()
{
	return static_cast<bool>(glfwWindowShouldClose(window.getHandle()));
}

bool MultiRenderer::isDeviceChosen(VkPhysicalDevice device)
{
	auto found = find_if(devices.begin(), devices.end(),
			     [device](Device& other) -> bool{
				     return other.getPhysicalHandle() == device;
			     });
	return found != devices.end();
}

vector<VkRect2D> MultiRenderer::calcululateSubRendererAreas(uint32_t width, uint32_t height)
{
	if (strategy == Strategy::SORT_LAST)
	{
		return vector<VkRect2D>(deviceCount, {{}, {width, height}});
	}

	uint32_t sliceHeight = height / deviceCount;
	vector<VkRect2D> result;
	for (uint32_t i = 0; i < deviceCount; i++)
		result.push_back({{0, i * sliceHeight}, {width, sliceHeight}});
	result[deviceCount - 1].extent.height += height - sliceHeight * deviceCount;
	return result;
}

vector<pair<uint32_t, uint32_t>> MultiRenderer::calculateMeshPortions()
{
	if (strategy == Strategy::SORT_FIRST)
	{
		return vector<pair<uint32_t, uint32_t>>(
			deviceCount,
			pair<uint32_t, uint32_t>(0, mesh->getElementCount()));
	}

	uint32_t meshPortion = mesh->getElementCount() / deviceCount;
	meshPortion -= meshPortion % 3;
	uint32_t firstMeshPortion = meshPortion
				    + (mesh->getElementCount() - meshPortion * deviceCount);
	uint32_t offset = 0;

	vector<pair<uint32_t, uint32_t>> result;
	result.reserve(deviceCount);
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		uint32_t elementCount = i == 0 ? firstMeshPortion : meshPortion;
		result.emplace_back(offset, elementCount);
		offset += elementCount;
	}
	return result;
}