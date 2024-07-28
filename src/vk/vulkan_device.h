#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <optional>

#include "physical_device.h"

class VulkanContext;

class VulkanDevice
{
public:
    VulkanDevice(VulkanContext &context, PhysicalDevice &device_info);
    void destroy();

    PhysicalDevice &get_physical_device() { return this->physical_device_info; }
    VkDevice get_device() { return this->logical_device; }

    uint32_t get_graphics_family() { return this->graphics_family; }
    uint32_t get_transfer_family() { return this->transfer_family; }
    uint32_t get_present_family() { return this->present_family; }

private:
    void create_logical_device();

    VulkanContext &context;
    VkDevice logical_device;
    PhysicalDevice physical_device_info;

    uint32_t graphics_family;
    VkQueue graphics_queue;

    uint32_t transfer_family;
    VkQueue transfer_queue;

    uint32_t present_family;
    VkQueue present_queue;
};