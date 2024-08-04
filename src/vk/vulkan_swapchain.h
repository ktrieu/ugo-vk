#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Window;
class VulkanContext;

class VulkanSwapchain
{
public:
	VulkanSwapchain(VulkanContext &context, Window &window);

	VkSwapchainKHR get_swapchain() { return this->swapchain; }
	VkFormat get_surface_format() { return this->surface_format; }

	uint32_t acquire_image(VkSemaphore completion);
	VkImage get_swapchain_image(uint32_t idx);

	void destroy();

private:
	VulkanContext &context;

	VkSwapchainKHR swapchain;
	bool image_sharing_required;

	std::vector<VkImage> images;
	VkFormat surface_format;
	VkExtent2D swap_extent;
	std::vector<VkImageView> image_views;

	VkSurfaceFormatKHR select_format();
	VkPresentModeKHR select_present_mode();
	VkExtent2D choose_swap_extent(Window &window);

	VkImageView create_image_view(VkImage image);
};