#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct PhysicalDeviceInfo {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties2 properties;
    VkPhysicalDeviceFeatures2 features;
    std::vector<VkExtensionProperties> extensions;
    std::vector<VkQueueFamilyProperties2> queue_families;

    PhysicalDeviceInfo(VkPhysicalDevice device);

    std::vector<uint32_t> get_queue_families_for_type(VkQueueFlags ty);

    bool is_usable();
};