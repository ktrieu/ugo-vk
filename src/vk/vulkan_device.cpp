#include "vulkan_device.h"

#include <algorithm>
#include <stdexcept>
#include <array>

#include <fmt/format.h>

#include "vulkan_error.h"
#include "logger.h"

VulkanDevice::VulkanDevice(VulkanContext &context, PhysicalDeviceInfo& device_info) : context(context), physical_device_info(device_info)
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

	std::optional<uint32_t> graphics_family_idx = this->physical_device_info.get_graphics_family();

	if (!graphics_family_idx.has_value())
	{
		throw std::runtime_error("No graphics queue family available.");
	}

	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueCount = 1;
	queue_create_info.queueFamilyIndex = graphics_family_idx.value();
	float priority = 1.0f;
	queue_create_info.pQueuePriorities = &priority;
	
	info.queueCreateInfoCount = 1;
	info.pQueueCreateInfos = &queue_create_info;

	VkResult result = vkCreateDevice(this->physical_device_info.get_device(), &info, nullptr, &this->logical_device);
	vk_check(result);

	VkDeviceQueueInfo2 queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	queue_info.queueFamilyIndex = graphics_family_idx.value();
	queue_info.queueIndex = 0;
	vkGetDeviceQueue2(this->logical_device, &queue_info, &this->graphics_queue);
}
