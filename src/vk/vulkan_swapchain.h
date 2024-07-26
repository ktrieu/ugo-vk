#pragma once

#include <vulkan/vulkan.h>

class Window;
class VulkanContext;

class VulkanSwapchain
{
public:
	VulkanSwapchain(VulkanContext &context, Window &window);

	void destroy();

private:
	VulkanContext &context;

	VkSwapchainKHR swapchain;
	bool image_sharing_required;

	VkSurfaceFormat2KHR select_format();
	VkPresentModeKHR select_present_mode();
	VkExtent2D choose_swap_extent(Window &window);
};