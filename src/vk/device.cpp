#include "device.h"

#include <algorithm>
#include <stdexcept>
#include <array>
#include <unordered_set>

#include <fmt/format.h>

#include "vulkan_error.h"
#include "logger.h"

vk::Device::Device(vk::Context &context, PhysicalDevice &device_info) : _context(context), _physical_device(device_info)
{
	this->create_logical_device();
}

void vk::Device::destroy()
{
	vkDestroyDevice(this->_device, nullptr);
}

VkCommandPool alloc_command_pool(VkDevice device, uint32_t queue_index, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	info.queueFamilyIndex = queue_index;
	info.flags = flags;

	VkCommandPool pool;
	auto result = vkCreateCommandPool(device, &info, nullptr, &pool);
	vk_check(result);

	return pool;
}

VkCommandPool vk::Device::alloc_graphics_pool(VkCommandPoolCreateFlags flags)
{
	return alloc_command_pool(this->_device, this->graphics_family(), flags);
}

VkCommandPool vk::Device::alloc_transfer_pool(VkCommandPoolCreateFlags flags)
{
	return alloc_command_pool(this->_device, this->transfer_family(), flags);
}

void vk::Device::create_logical_device()
{
	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// We don't really care about features for now.
	VkPhysicalDeviceFeatures2 device_features = {};
	device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	info.pEnabledFeatures = &device_features.features;

	// Dynamic rendering is hidden behind it's own feature flag struct.
	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;
	info.pNext = &dynamic_rendering_features;

	// As is synchronization2.
	VkPhysicalDeviceSynchronization2Features sync_features = {};
	sync_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync_features.synchronization2 = VK_TRUE;
	dynamic_rendering_features.pNext = &sync_features;

	info.enabledExtensionCount = PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS.size();
	info.ppEnabledExtensionNames = PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS.data();

	std::optional<uint32_t> graphics_family_idx = this->_physical_device.get_graphics_family();
	if (!graphics_family_idx.has_value())
	{
		throw std::runtime_error("No graphics queue family available.");
	}
	this->_graphics_family = graphics_family_idx.value();

	std::optional<uint32_t> present_family_idx = this->_physical_device.get_present_family();
	if (!present_family_idx.has_value())
	{
		throw std::runtime_error("No present queue family available.");
	}
	this->_present_family = present_family_idx.value();

	std::optional<uint32_t> transfer_family_idx = this->_physical_device.get_transfer_family();
	if (!transfer_family_idx.has_value())
	{
		throw std::runtime_error("No transfer family available.");
	}
	this->_transfer_family = transfer_family_idx.value();

	// Queue families may overlap so use a set to distinguish them.
	std::unordered_set<uint32_t> required_queues = {this->_graphics_family, this->_transfer_family, this->_present_family};

	std::vector<VkDeviceQueueCreateInfo> queue_infos;
	float priority = 1.0f;

	for (auto idx : required_queues)
	{
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueCount = 1;
		queue_create_info.queueFamilyIndex = idx;
		queue_create_info.pQueuePriorities = &priority;

		queue_infos.push_back(queue_create_info);
	}

	info.queueCreateInfoCount = queue_infos.size();
	info.pQueueCreateInfos = queue_infos.data();

	VkResult result = vkCreateDevice(this->_physical_device.get_device(), &info, nullptr, &this->_device);
	vk_check(result);

	VkDeviceQueueInfo2 graphics_queue_info = {};
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	graphics_queue_info.queueFamilyIndex = this->_graphics_family;
	graphics_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->_device, &graphics_queue_info, &this->_graphics_queue);

	VkDeviceQueueInfo2 present_queue_info = {};
	present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	present_queue_info.queueFamilyIndex = this->_present_family;
	present_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->_device, &present_queue_info, &this->_present_queue);

	VkDeviceQueueInfo2 transfer_queue_info = {};
	transfer_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	transfer_queue_info.queueFamilyIndex = this->_transfer_family;
	transfer_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->_device, &transfer_queue_info, &this->_transfer_queue);
}
