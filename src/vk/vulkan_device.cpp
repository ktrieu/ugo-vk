#include "vulkan_device.h"

#include <algorithm>
#include <stdexcept>
#include <array>
#include <unordered_set>

#include <fmt/format.h>

#include "vulkan_error.h"
#include "logger.h"

VulkanDevice::VulkanDevice(VulkanContext &context, PhysicalDevice &device_info) : context(context), physical_device_info(device_info)
{
	this->create_logical_device();
}

void VulkanDevice::destroy()
{
	vkDestroyDevice(this->logical_device, nullptr);
}

void VulkanDevice::create_logical_device()
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

	info.enabledExtensionCount = PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS.size();
	info.ppEnabledExtensionNames = PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS.data();

	std::optional<uint32_t> graphics_family_idx = this->physical_device_info.get_graphics_family();
	if (!graphics_family_idx.has_value())
	{
		throw std::runtime_error("No graphics queue family available.");
	}
	this->graphics_family = graphics_family_idx.value();

	std::optional<uint32_t> present_family_idx = this->physical_device_info.get_present_family();
	if (!present_family_idx.has_value())
	{
		throw std::runtime_error("No present queue family available.");
	}
	this->present_family = present_family_idx.value();

	std::optional<uint32_t> transfer_family_idx = this->physical_device_info.get_transfer_family();
	if (!transfer_family_idx.has_value())
	{
		throw std::runtime_error("No transfer family available.");
	}
	this->transfer_family = transfer_family_idx.value();

	// Queue families may overlap so use a set to distinguish them.
	std::unordered_set<uint32_t> required_queues = {this->graphics_family, this->transfer_family, this->present_family};

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

	VkResult result = vkCreateDevice(this->physical_device_info.get_device(), &info, nullptr, &this->logical_device);
	vk_check(result);

	VkDeviceQueueInfo2 graphics_queue_info = {};
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	graphics_queue_info.queueFamilyIndex = this->graphics_family;
	graphics_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->logical_device, &graphics_queue_info, &this->graphics_queue);

	VkDeviceQueueInfo2 present_queue_info = {};
	present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	present_queue_info.queueFamilyIndex = this->present_family;
	present_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->logical_device, &present_queue_info, &this->present_queue);

	VkDeviceQueueInfo2 transfer_queue_info = {};
	transfer_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	transfer_queue_info.queueFamilyIndex = this->transfer_family;
	transfer_queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->logical_device, &transfer_queue_info, &this->transfer_queue);
}
