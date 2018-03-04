#include "SortFirstBorderlessRenderer.h"
#include <future>

using namespace bp;
using namespace std;

void SortFirstBorderlessRenderer::init(Instance& instance, uint32_t width, uint32_t height,
				       bpScene::Mesh& mesh, Scene& scene)
{
	SortFirstBorderlessRenderer::instance = &instance;
	SortFirstBorderlessRenderer::mesh = &mesh;
	SortFirstBorderlessRenderer::scene = &scene;

	//Find the devices
	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.extensions.push_back("VK_KHR_swapchain");

	physicalDevices = queryDevices(instance, requirements);
	auto count = physicalDevices.size();
	while (physicalDevices.size() < deviceCount)
	{
		for (auto i = 0; i < count && physicalDevices.size() < deviceCount; i++)
			physicalDevices.push_back(physicalDevices[i]);
	}
	if (physicalDevices.size() > deviceCount)
		physicalDevices.resize(deviceCount);

	areas = calcululateSubRendererAreas(width, height);

	subRenderers.resize(deviceCount);
	subpasses.resize(deviceCount);

	for (auto i = 0; i < deviceCount; i++)
	{
		subpasses[i].setArea(
			static_cast<float>(areas[i].offset.x) / static_cast<float>(width),
			static_cast<float>(areas[i].offset.y) / static_cast<float>(height),
			static_cast<float>(areas[i].extent.width) / static_cast<float>(width),
			static_cast<float>(areas[i].extent.height) / static_cast<float>(height)
		);
		subRenderers[i].init(instance, physicalDevices[i], subpasses[i], areas[i]);
	}
}

void SortFirstBorderlessRenderer::render()
{
	vector<future<void>> futures;
	for (auto& r : subRenderers)
		futures.push_back(async(launch::async, [&r]{ r.render(); }));
	for (auto& fut : futures) fut.wait();

	for (auto& r : subRenderers)
		r.present();
}

bool SortFirstBorderlessRenderer::shouldClose()
{
	for (auto& r : subRenderers)
		if (glfwWindowShouldClose(r.window.getHandle())) return true;
	return false;
}

vector<VkRect2D> SortFirstBorderlessRenderer::calcululateSubRendererAreas(uint32_t width,
									  uint32_t height)
{
	uint32_t sliceHeight = height / deviceCount;
	vector<VkRect2D> result;
	for (uint32_t i = 0; i < deviceCount; i++)
		result.push_back({{0, i * sliceHeight}, {width, sliceHeight}});
	result[deviceCount - 1].extent.height += height - sliceHeight * deviceCount;
	return result;
}