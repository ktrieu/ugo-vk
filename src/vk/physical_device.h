#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string_view>
#include <optional>

class PhysicalDevice {
public:
    PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool is_usable();

    std::string_view get_name();
    VkPhysicalDevice get_device() { return this->device; }

    std::optional<uint32_t> get_graphics_family();
    std::optional<uint32_t> get_transfer_family();
    std::optional<uint32_t> get_present_family();

    static const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS;

private:
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties2 properties;
    VkPhysicalDeviceFeatures2 features;
    std::vector<VkExtensionProperties> extensions;
    std::vector<VkQueueFamilyProperties2> queue_families;

    std::vector<uint32_t> graphics_families;
    std::vector<uint32_t> transfer_families;
    std::vector<uint32_t> present_families;

    std::vector<uint32_t> get_queue_families_for_type(VkQueueFlags ty);
    std::vector<uint32_t> get_present_families(VkSurfaceKHR surface);
};