#include "sync.h"

#include "device.h"
#include "vulkan_error.h"

vk::Semaphore::Semaphore(vk::Device& device, VkSemaphoreCreateFlags flags) : _device(device)
{
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    info.flags = flags;

    auto result = vkCreateSemaphore(device.device(), &info, nullptr, &_semaphore);
    vk_check(result);
}

vk::Semaphore::~Semaphore()
{
    vkDestroySemaphore(_device.device(), _semaphore, nullptr);
}

VkSemaphoreSubmitInfo vk::Semaphore::submit_info(VkPipelineStageFlags2 stages)
{
    VkSemaphoreSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

    info.semaphore = _semaphore;
    info.stageMask = stages;

    info.deviceIndex = 0;
    info.value = 1;

    return info;
}

vk::Fence::Fence(vk::Device& device, VkFenceCreateFlags flags) : _device(device)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = flags;

    auto result = vkCreateFence(_device.device(), &info, nullptr, &_fence);
    vk_check(result);
}

vk::Fence::~Fence()
{
    vkDestroyFence(_device.device(), _fence, nullptr);
}

void vk::Fence::wait(uint64_t timeout_ns)
{
    auto result = vkWaitForFences(_device.device(), 1, &_fence, true, timeout_ns);
    vk_check(result);
}

void vk::Fence::reset()
{
    auto result = vkResetFences(_device.device(), 1, &_fence);
    vk_check(result);
}