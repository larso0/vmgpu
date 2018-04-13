#include "Vmgpu.h"
#include "SFRenderer.h"
#include <bpMulti/SortFirstCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::initSortFirst(uint32_t width, uint32_t height)
{
	vector<pair<Device*, SortFirstRenderer*>> configurations;
	for (auto& device : devices)
	{
		SFRenderer* renderer = new SFRenderer();
		renderer->setCamera(camera);
		renderers.emplace_back(renderer);
		configurations.emplace_back(device.get(), renderer);
	}

	if (devices.size() > 1)
	{
		SortFirstCompositor* compositor = new SortFirstCompositor();
		compositor->init(move(configurations), swapchain.getFormat(), width, height);
		mainRenderer.reset(compositor);
	} else
	{
		mainRenderer = renderers[0];
		mainRenderer->init(device, swapchain.getFormat(), width, height);
	}

	for (auto& r : renderers)
	{
		auto renderer = static_pointer_cast<SFRenderer>(r);
		auto& rm = renderer->getResourceManager();
		if (options.basic)
		{
			unsigned meshId = rm.addMesh(scene.mesh);
			rm.addMeshInstance(meshId, scene.node);
		} else
		{
			unsigned modelId = rm.addModel(scene.model);
			rm.addModelInstance(modelId, scene.node);
		}
	}
}