#pragma once

#include <vulkan/vulkan.h>

namespace vk {
	VkImageSubresourceRange get_image_range(VkImageAspectFlags aspect);

	struct ImageBarrierState {
		VkImageLayout layout;
		VkPipelineStageFlags stage;
		VkAccessFlags2 access;
	};

	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageSubresourceRange range, ImageBarrierState old_state, ImageBarrierState new_state);
}