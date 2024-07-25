#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <optional>

#include "physical_device.h"

class VulkanContext;

class VulkanDevice
{
public:
    VulkanDevice(VulkanContext &context, PhysicalDeviceInfo& device_info);
    void destroy();

private:
    void create_logical_device();

    VulkanContext &context;
    VkDevice logical_device;
    PhysicalDeviceInfo physical_device_info;

    VkQueue graphics_queue;
    VkQueue present_queue;
};