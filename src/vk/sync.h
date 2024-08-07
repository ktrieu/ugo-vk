#pragma once

#include <vulkan/vulkan.h>

namespace vk {

class Device;

class Semaphore {
public:
	Semaphore(vk::Device& device, VkSemaphoreCreateFlags flags);
	~Semaphore();

	Semaphore& operator=(const Semaphore& other) = delete;
	Semaphore(const Semaphore& other) = delete;

	VkSemaphore vk_semaphore() { return _semaphore; }
	VkSemaphoreSubmitInfo submit_info(VkPipelineStageFlags2 stages);

private:
	vk::Device& _device;
	VkSemaphore _semaphore;
};

class Fence {
public:
	Fence(vk::Device& device, VkFenceCreateFlags flags);
	~Fence();

	Fence& operator=(const Fence& other) = delete;
	Fence(const Fence& other) = delete;

	VkFence vk_fence() { return _fence; }

	void wait(uint64_t timeout_ns);
	void reset();

private:
	vk::Device& _device;
	VkFence _fence;
};

}