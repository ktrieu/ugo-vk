#pragma once

#include <vulkan/vulkan.h>

namespace vk {
	class CommandBuffer {
	public:
		CommandBuffer(VkDevice device, VkCommandPool pool);

		VkCommandBuffer buffer() { return _buffer; }

		void begin(VkCommandBufferUsageFlags flags);
		void end();

		VkCommandBufferSubmitInfo submit_info();

	private:
		VkDevice _device;
		VkCommandBuffer _buffer;
	};
}