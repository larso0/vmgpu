#include "Vmgpu.h"
#include <set>
#include <stdexcept>

using namespace bp;
using namespace std;

void Vmgpu::initRenderResources(uint32_t width, uint32_t height)
{
	//Select devices to use for rendering

	//First device both renders and takes care of compositing
	//Using device provided by the window
	devices.emplace_back(&device, [](Device*){});

	//Select secondary devices if requested
	if (options.deviceCount > 1)
	{
		DeviceRequirements requirements;
		requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
		requirements.features.samplerAnisotropy = VK_TRUE;

		auto physicalDevices = bpQt::queryDevices(*vulkanInstance(), requirements);
		set<VkPhysicalDevice> selected;
		selected.insert(device);
		for (VkPhysicalDevice d : physicalDevices)
		{
			if (selected.size() == options.deviceCount) break;
			if (selected.find(d) == selected.end()) selected.insert(d);
		}

		if (selected.size() < options.deviceCount)
			qWarning() << options.deviceCount << " devices requested, but only " << selected.size() << " devices are available.";

		selected.erase(device);
		for (VkPhysicalDevice d : selected)
			devices.emplace_back(new Device(d, requirements));
	}

	mesh.loadObj(options.objPath);

	switch (options.strategy)
	{
	case Strategy::SortFirst: initSortFirst(width, height); break;
	case Strategy::Single:
	case Strategy::SortLast: initSortLast(width, height); break;
	default: throw runtime_error("Unsupported strategy.");
	}
}

void Vmgpu::initSortFirst(uint32_t width, uint32_t height)
{

}

void Vmgpu::initSortLast(uint32_t width, uint32_t height)
{

}

void Vmgpu::specifyDeviceRequirements(DeviceRequirements& requirements)
{
	requirements.features.samplerAnisotropy = VK_TRUE;
}
