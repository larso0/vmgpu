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
			qWarning() << options.deviceCount << " devices requested, but only "
				   << selected.size() << " devices are available.";

		selected.erase(device);
		for (VkPhysicalDevice d : selected)
			devices.emplace_back(new Device(d, requirements));
	}

	qInfo() << "Loading \"" << options.objPath.c_str() << "\"...";
	mesh.loadObj(options.objPath);
	cameraNode.translate(0.f, 0.f, 2.f);
	cameraNode.update();
	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, 100.f);
	camera.update();

	qInfo() << "Initializing renderer...";
	switch (options.strategy)
	{
	case Strategy::Single:
	case Strategy::SortFirst: initSortFirst(width, height); break;
	//case Strategy::SortLast: initSortLast(width, height); break;
	default: throw runtime_error("Unsupported strategy.");
	}

	qInfo() << "Rendering...";
}

void Vmgpu::resizeRenderResources(uint32_t width, uint32_t height)
{
	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, 100.f);
	camera.update();
	mainRenderer->resize(width, height);
	Window::resizeRenderResources(width, height);
}

void Vmgpu::specifyDeviceRequirements(DeviceRequirements& requirements)
{
	requirements.features.samplerAnisotropy = VK_TRUE;
}

void Vmgpu::render(VkCommandBuffer cmdBuffer)
{
	mainRenderer->render(framebuffer, cmdBuffer);
}

void Vmgpu::update(double frameDeltaTime)
{
	objectNode.rotate(static_cast<float>(frameDeltaTime), {0.f, 1.f, 0.f});
	objectNode.update();
}