#include "vulkan/vulkan.h"

class VulkanContext;

class VulkanDevice
{
public:
    VulkanDevice(VulkanContext &context, VkPhysicalDevice physical_device);

private:
    VulkanContext &context;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
};