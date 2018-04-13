#include "Vmgpu.h"
#include "SLRenderer.h"
#include <bpMulti/SortLastCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::initSortLast(uint32_t width, uint32_t height)
{
	vector<pair<Device*, SortLastRenderer*>> configurations;
	for (auto& device : devices)
	{
		SLRenderer* renderer = new SLRenderer();
		renderer->setCamera(camera);
		renderers.emplace_back(renderer);
		configurations.emplace_back(device.get(), renderer);
	}

	SortLastCompositor* compositor = new SortLastCompositor();
	compositor->init(move(configurations), swapchain.getFormat(), width, height);
	mainRenderer.reset(compositor);

/*	uint32_t count = static_cast<uint32_t>(mesh.getElementCount() / devices.size());
	count -= count % 3;
	uint32_t leftover = static_cast<uint32_t>(mesh.getElementCount() - count * devices.size());
	for (unsigned i = 0; i < devices.size(); i++)
	{
		auto renderer = static_pointer_cast<SLRenderer>(renderers[i]);
		unsigned meshId = renderer->addMesh(mesh, i * count,
						    i == devices.size() - 1 ? count + leftover
									    : count);
		renderer->addEntity(meshId, objectNode);
	}*/
	throw runtime_error("Sort-last not implemented yet.");
}
