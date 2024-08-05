#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice;

namespace vk {

class Semaphore {
public:
	Semaphore(VulkanDevice& device, VkSemaphoreCreateFlags flags);
	~Semaphore();

	VkSemaphore vk_semaphore() { return _semaphore; }
	VkSemaphoreSubmitInfo submit_info(VkPipelineStageFlags2 stages);

private:
	VulkanDevice& _device;
	VkSemaphore _semaphore;
};

class Fence {
public:
	Fence(VulkanDevice& device, VkFenceCreateFlags flags);
	~Fence();

	VkFence vk_fence() { return _fence; }

	void wait(uint64_t timeout_ns);
	void reset();

private:
	VulkanDevice& _device;
	VkFence _fence;
};

}