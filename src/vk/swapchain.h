#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Window;

namespace vk {

	class Semaphore;
	class Context;

	class Swapchain {
	public:
		Swapchain(vk::Context &context, Window &window);

		VkSwapchainKHR swapchain() { return _swapchain; }
		VkFormat surface_format() { return _surface_format; }

		uint32_t acquire_image(vk::Semaphore& completion);

		VkImage get_swapchain_image(uint32_t idx);
		VkImageView get_swapchain_image_view(uint32_t idx);
		VkExtent2D get_swap_extent() { return _swap_extent; }

		void present(uint32_t idx, VkQueue queue, vk::Semaphore& completion);

		void destroy();

	private:
		vk::Context& _context;

		VkSwapchainKHR _swapchain;
		bool _image_sharing_required;

		std::vector<VkImage> _images;
		VkFormat _surface_format;
		VkExtent2D _swap_extent;
		std::vector<VkImageView> _image_views;

		VkSurfaceFormatKHR select_format();
		VkPresentModeKHR select_present_mode();
		VkExtent2D choose_swap_extent(Window &window);

		VkImageView create_image_view(VkImage image);
	};

}
