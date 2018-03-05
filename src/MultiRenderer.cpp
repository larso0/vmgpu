#include "MultiRenderer.h"
#include <algorithm>
#include <future>
#include <iostream>
#include <iomanip>

using namespace bp;
using namespace bpUtil;
using namespace std;

void MultiRenderer::init(Instance& instance, uint32_t width, uint32_t height, bpScene::Mesh& mesh)
{
	MultiRenderer::instance = &instance;
	MultiRenderer::mesh = &mesh;

	devices.reserve(deviceCount);
	secondaryRenderers.reserve(deviceCount - 1);
	subpasses.reserve(deviceCount);
	compositingColorSources.reserve(deviceCount);
	compositingDepthSources.reserve(deviceCount);

	//Setup scene
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2000.f);
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

	compositingSubpass.addColorAttachment(swapchain);

	if (strategy == Strategy::SORT_LAST)
	{
		depthAttachment.setClearEnabled(true);
		depthAttachment.setClearValue({1.f, 0.f});
		depthAttachment.init(devices[0], VK_FORMAT_D16_UNORM,
				     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);
		compositingSubpass.setDepthTestEnabled(true);
		compositingSubpass.setDepthAttachment(depthAttachment);
	}

	compositingRenderPass.addSubpassGraph(compositingSubpass);

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

	const auto& area = renderAreas[0];

	//Setup resources for the first GPU
	renderColorAttachment.setClearEnabled(true);
	renderColorAttachment.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	renderColorAttachment.init(devices[0], VK_FORMAT_R8G8B8A8_UNORM,
				   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				   area.extent.width, area.extent.height);
	renderDepthAttachment.setClearEnabled(true);
	renderDepthAttachment.setClearValue({1.f, 0.f});
	renderDepthAttachment.init(devices[0], VK_FORMAT_D16_UNORM,
				   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				   area.extent.width, area.extent.height);
	subpasses.emplace_back();
	subpasses[0].setScene(mesh, portions[0].first, portions[0].second, meshNode, camera);
	subpasses[0].setClipTransform(
		static_cast<float>(area.offset.x) / static_cast<float>(width),
		static_cast<float>(area.offset.y) / static_cast<float>(height),
		static_cast<float>(area.extent.width) / static_cast<float>(width),
		static_cast<float>(area.extent.height) / static_cast<float>(height)
	);
	subpasses[0].addColorAttachment(renderColorAttachment);
	subpasses[0].setDepthAttachment(renderDepthAttachment);
	renderPass.addSubpassGraph(subpasses[0]);
	renderPass.setRenderArea({{}, {area.extent.width, area.extent.height}});
	renderPass.init(area.extent.width, area.extent.height);

	//Setup resources for secondary renderers
	for (uint32_t i = 1; i < deviceCount; i++)
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

		secondaryRenderers.emplace_back();
		secondaryRenderers[i - 1].init(strategy, devices[i], renderAreas[i].extent.width,
					       renderAreas[i].extent.height, subpasses[i]);
	}

	//Setup textures for compositing
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		uint32_t w = renderAreas[i].extent.width;
		uint32_t h = renderAreas[i].extent.height;
		compositingColorSources.emplace_back(devices[0], VK_FORMAT_R8G8B8A8_UNORM,
						     VK_IMAGE_USAGE_SAMPLED_BIT, w, h);
		compositingColorSources[i].getImage().map();
		if (strategy == Strategy::SORT_LAST)
		{
			compositingDepthSources.emplace_back(devices[0], VK_FORMAT_D16_UNORM,
							     VK_IMAGE_USAGE_SAMPLED_BIT, w, h);
			compositingDepthSources[i].getImage().map();
			compositingSubpass.addTexture(renderAreas[i], compositingColorSources[i],
						      compositingDepthSources[i]);
		} else
		{
			compositingSubpass.addTexture(renderAreas[i], compositingColorSources[i]);
		}
	}

	//Handle window resize events
	connect(window.resizeEvent, swapchain, &Swapchain::resize);

	//Resize resources when the swapchain is resized
	connect(swapchain.resizeEvent, *this, &MultiRenderer::resize);

	//Initialize resources for compositing
	compositingRenderPass.init(width, height);
	compositingRenderPass.setRenderArea({{}, {width, height}});

	cmdPool.init(devices[0].getGraphicsQueue());
	frameCompleteSem.init(devices[0]);
	frameCmdBuffer = cmdPool.allocateCommandBuffer();

	//Render the first frame
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(frameCmdBuffer, &beginInfo);
	recordRender();
	vkEndCommandBuffer(frameCmdBuffer);

	queue->submit({}, {frameCmdBuffer}, {});
	queue->waitIdle();
}

void MultiRenderer::setColor(uint32_t deviceIndex, const glm::vec3& color)
{
	subpasses[deviceIndex].setColor(color);
}

void MultiRenderer::render()
{
	window.handleEvents();

	vector<future<void>> renderFutures;
	vector<future<void>> copyFutures;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	auto primaryRenderFuture = async(launch::async, [this, &beginInfo]{
		double t0 = glfwGetTime();
		vkBeginCommandBuffer(frameCmdBuffer, &beginInfo);
		if (resized)
		{
			recordRender();
			recordCopy();
			resized = false;
		} else
		{
			recordCopy();
			recordRender();
		}
		vkEndCommandBuffer(frameCmdBuffer);
		queue->submit({}, {frameCmdBuffer}, {});
		queue->waitIdle();
		double t1 = glfwGetTime();
		unique_lock<mutex> lock(measureMut);
		measureAccumulators["renderAndCopy0"] += t1 - t0;
	});

	for (int i = 1; i < deviceCount; i++)
	{
		auto& r = secondaryRenderers[i - 1];
		r.selectStagingBuffers();

		renderFutures.push_back(async(launch::async, [&r, this, i]{
			double t0 = glfwGetTime();
			r.render();
			double t1 = glfwGetTime();
			{
				unique_lock<mutex> lock(measureMut);
				measureAccumulators["render" + to_string(i)] += t1 - t0;
			}
		}));

		copyFutures.push_back(async(launch::async, [&r, this, i]{
			double t0 = glfwGetTime();
			r.copy(compositingColorSources[i].getImage().map(),
			       strategy == Strategy::SORT_LAST
			       ? compositingDepthSources[i].getImage().map() : nullptr);
			double t1 = glfwGetTime();
			{
				unique_lock<mutex> lock(measureMut);
				measureAccumulators["copy" + to_string(i)] += t1 - t0;
			}
		}));
	}
	for (auto& f : copyFutures) f.wait();
	primaryRenderFuture.wait();

	double t0 = glfwGetTime();
	vkBeginCommandBuffer(frameCmdBuffer, &beginInfo);
	recordCompositing();
	vkEndCommandBuffer(frameCmdBuffer);

	queue->submit({{swapchain.getImageAvailableSemaphore(),
			       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		      {frameCmdBuffer}, {frameCompleteSem});
	queue->waitIdle();
	swapchain.present(frameCompleteSem);
	double t1 = glfwGetTime();

	measureAccumulators["composite"] += t1 - t0;

	measureFrameCount++;

	for (auto& f : renderFutures) f.wait();
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

void MultiRenderer::printMeasurements()
{
	if (measureFrameCount > 1)
	{
		for (auto a : measureAccumulators)
			measureAccumulators[a.first] = a.second / measureFrameCount;
		measureFrameCount = 0;
	}

	auto iter = measureAccumulators.begin();
	cout << iter->first << " = " << setprecision(4) << iter->second;
	iter++;
	for (; iter != measureAccumulators.end(); iter++)
		cout  << ", " << iter->first << " = " << setprecision(4) << iter->second;
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
		result.push_back({{0, static_cast<int32_t>(i * sliceHeight)},
				  {width, sliceHeight}});
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

void MultiRenderer::resize(uint32_t w, uint32_t h)
{
	if (strategy == Strategy::SORT_LAST) depthAttachment.resize(w, h);
	compositingRenderPass.setRenderArea({{}, {w, h}});
	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);

	auto renderAreas = calcululateSubRendererAreas(w, h);
	renderColorAttachment.resize(renderAreas[0].extent.width, renderAreas[0].extent.height);
	renderDepthAttachment.resize(renderAreas[0].extent.width, renderAreas[0].extent.height);
	renderPass.resize(renderAreas[0].extent.width, renderAreas[0].extent.height);
	renderPass.setRenderArea({{}, renderAreas[0].extent});

	resized = true;

	for (auto i = 0; i < deviceCount; i++)
	{
		const auto& area = renderAreas[i];
		subpasses[i].setClipTransform(
			static_cast<float>(area.offset.x) / static_cast<float>(w),
			static_cast<float>(area.offset.y) / static_cast<float>(h),
			static_cast<float>(area.extent.width) / static_cast<float>(w),
			static_cast<float>(area.extent.height) / static_cast<float>(h)
		);
		if (i > 0)
		{
			secondaryRenderers[i - 1].resize(area.extent.width,
							 area.extent.height);
		}
		compositingColorSources[i].resize(area.extent.width, area.extent.height);
		compositingColorSources[i].getImage().map();
		if (strategy == Strategy::SORT_LAST)
		{
			compositingDepthSources[i].resize(area.extent.width,
							  area.extent.height);
			compositingDepthSources[i].getImage().map();
		}
		compositingSubpass.resizeTextureResources(i, area);
	}
}

void MultiRenderer::recordCopy()
{
	compositingColorSources[0].getImage().transfer(renderColorAttachment.getImage(),
						       frameCmdBuffer);
	if (strategy == Strategy::SORT_LAST)
	{
		compositingDepthSources[0].getImage().transfer(renderDepthAttachment.getImage(),
							       frameCmdBuffer);
	}
}

void MultiRenderer::recordRender()
{
	renderPass.render(frameCmdBuffer);
}

void MultiRenderer::recordCompositing()
{
	compositingColorSources[0].transitionShaderReadable(frameCmdBuffer,
							    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	if (strategy == Strategy::SORT_LAST)
	{
		compositingDepthSources[0].transitionShaderReadable(
			frameCmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}
	for (auto i = 1; i < deviceCount; i++)
	{
		compositingColorSources[i].getImage().flushStagingBuffer(frameCmdBuffer);
		compositingColorSources[i].transitionShaderReadable(
			frameCmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		if (strategy == Strategy::SORT_LAST)
		{
			compositingDepthSources[i].getImage().flushStagingBuffer(frameCmdBuffer);
			compositingDepthSources[i].transitionShaderReadable(
				frameCmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
	}

	compositingRenderPass.render(frameCmdBuffer);
}