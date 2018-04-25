#include "Vmgpu.h"
#include "SFRenderer.h"
#include <bpMulti/SortFirstCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::initSortFirst(uint32_t width, uint32_t height)
{
	loadMessageEvent("Initializing renderers...");
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
		loadMessageEvent("Initializing sort-first compositor...");
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
		bpUtil::connect(rm.loadMessageEvent, loadMessageEvent);
		if (options.basic)
		{
			for (const auto& mesh : scene.meshes)
			{
				unsigned meshId = rm.addMesh(mesh);
				rm.addMeshInstance(meshId, scene.node);
			}
		} else
		{
			for (const auto& model : scene.models)
			{
				unsigned modelId = rm.addModel(model);
				rm.addModelInstance(modelId, scene.node);
			}
		}
	}
}