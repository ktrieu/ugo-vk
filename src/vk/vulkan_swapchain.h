#pragma once

#include <vulkan/vulkan.h>

class Window;
class VulkanContext;

class VulkanSwapchain
{
public:
	VulkanSwapchain(VulkanContext& context, Window& window);

	void destroy();
private:
	VulkanContext& context;

	VkSwapchainKHR swapchain;
};