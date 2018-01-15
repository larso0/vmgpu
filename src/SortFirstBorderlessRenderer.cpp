#include "SortFirstBorderlessRenderer.h"
#include <future>

using namespace bp;
using namespace std;

void SortFirstBorderlessRenderer::init(Instance& instance, uint32_t width, uint32_t height,
				       bpScene::Mesh& mesh)
{
	SortFirstBorderlessRenderer::instance = &instance;
	SortFirstBorderlessRenderer::mesh = &mesh;

	subRenderers.reserve(deviceCount);
	subpasses.reserve(deviceCount);

	//Setup scene
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.setPerspectiveProjection(glm::radians(60.f), aspectRatio, 0.01f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	sceneRoot.update();
	camera.update();

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

	auto portions = calculateMeshPortions();
	areas = calcululateSubRendererAreas(width, height);

	for (auto i = 0; i < deviceCount; i++)
	{
		subpasses[i].setScene(mesh, portions[i].first, portions[i].second, meshNode,
				      camera);
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

void SortFirstBorderlessRenderer::update(float delta)
{
	meshNode.rotate(delta, {0.f, 1.f, 0.f});
	meshNode.update();
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

vector<pair<uint32_t, uint32_t>> SortFirstBorderlessRenderer::calculateMeshPortions()
{
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