#include <iostream>
#include <bp/Instance.h>
#include <bp/Device.h>
#include <stdexcept>

using namespace bp;
using namespace std;

int main(int argc, char** argv)
{
	Instance instance;
	instance.enableExtension("VK_KHX_device_group_creation");
	instance.init(true);

	DeviceRequirements requirements{};
	requirements.extensions.emplace_back("VK_KHX_device_group");
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;

	auto result = queryDevices(instance, requirements);

	if (result.empty())
		throw runtime_error("No suitable devices.");

	PFN_vkEnumeratePhysicalDeviceGroupsKHX vkEnumeratePhysicalDeviceGroupsKHX =
		reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroupsKHX>(
			vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroupsKHX"));

	uint32_t n;
	vkEnumeratePhysicalDeviceGroupsKHX(instance, &n, nullptr);
	vector<VkPhysicalDeviceGroupPropertiesKHX> groupProperties(n);
	vkEnumeratePhysicalDeviceGroupsKHX(instance, &n, groupProperties.data());

	for (auto& group : groupProperties)
	{
		cout << "Group with " << group.physicalDeviceCount << " devices" << endl;
	}

	return 0;
}