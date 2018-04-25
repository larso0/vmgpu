#include "Vmgpu.h"
#include "SLRenderer.h"
#include <bpMulti/SortLastCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::initSortLast(uint32_t width, uint32_t height)
{
	loadMessageEvent("Initializing renderers...");
	vector<pair<Device*, SortLastRenderer*>> configurations;
	for (auto& device : devices)
	{
		SLRenderer* renderer = new SLRenderer();
		renderer->setCamera(camera);
		renderers.emplace_back(renderer);
		configurations.emplace_back(device.get(), renderer);
	}

	if (devices.size() > 1)
	{
		loadMessageEvent("Initializing sort-last compositor...");
		SortLastCompositor* compositor = new SortLastCompositor();
		compositor->init(move(configurations), swapchain.getFormat(), width, height);
		mainRenderer.reset(compositor);
	} else
	{
		mainRenderer = renderers[0];
		mainRenderer->init(device, swapchain.getFormat(), width, height);
	}

	for (auto& r : renderers)
	{
		auto& rm = static_pointer_cast<SLRenderer>(r)->getResourceManager();
		bpUtil::connect(rm.loadMessageEvent, loadMessageEvent);
	}

	if (options.objList && options.basic && scene.meshes.size() >= options.deviceCount)
	{
		for (unsigned i = 0; i < scene.meshes.size(); i++)
		{
			auto& renderer =
				*static_pointer_cast<SLRenderer>(renderers[i % renderers.size()]);
			auto& rm = renderer.getResourceManager();
			const auto& mesh = scene.meshes[i];
			unsigned meshId = rm.addMesh(mesh);
			rm.addMeshInstance(meshId, scene.node);
		}
	} else if (!options.basic && scene.models.size() >= options.deviceCount)
	{
		for (unsigned i = 0; i < scene.models.size(); i++)
		{
			auto& renderer =
				*static_pointer_cast<SLRenderer>(renderers[i % renderers.size()]);
			auto& rm = renderer.getResourceManager();
			const auto& model = scene.models[i];
			unsigned modelId = rm.addModel(model);
			rm.addModelInstance(modelId, scene.node);
		}
	} else if (options.basic)
	{
		const auto& mesh = scene.meshes[0];
		uint32_t count = static_cast<uint32_t>(mesh.getElementCount() / devices.size());
		count -= count % 3;
		uint32_t leftover = static_cast<uint32_t>(mesh.getElementCount() - count * devices.size());
		for (unsigned i = 0; i < devices.size(); i++)
		{
			auto& renderer =
				*static_pointer_cast<SLRenderer>(renderers[i % renderers.size()]);
			auto& rm = renderer.getResourceManager();
			unsigned meshId = rm.addMesh(mesh, i * count,
						     i == devices.size() - 1 ? count + leftover
									     : count);
			rm.addMeshInstance(meshId, scene.node);
		}
	} else
	{
		throw runtime_error("Unable to distribute the provided geometry");
	}
}
