#include "vulkan/vulkan.h"

#include <vector>
#include <optional>

class VulkanContext;

struct PhysicalDeviceInfo {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties2 properties;
    VkPhysicalDeviceFeatures2 features;
    std::vector<VkQueueFamilyProperties2> queue_families;

    PhysicalDeviceInfo(VkPhysicalDevice device);
};

class VulkanDevice
{
public:
    VulkanDevice(VulkanContext &context, PhysicalDeviceInfo& device_info);

private:
    void create_logical_device();

    VulkanContext &context;
    VkDevice logical_device;
    PhysicalDeviceInfo physical_device_info;

    VkQueue graphics_queue;
};