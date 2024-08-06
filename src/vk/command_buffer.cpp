#include "command_buffer.h"

#include "vulkan_error.h"

vk::CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool command_pool) : _device(device)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    info.commandPool = command_pool;
    info.commandBufferCount = 1;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto result = vkAllocateCommandBuffers(device, &info, &_buffer);
    vk_check(result);
}

void vk::CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    info.flags = flags;

    auto result = vkBeginCommandBuffer(_buffer, &info);
    vk_check(result);
}

void vk::CommandBuffer::end()
{
    auto result = vkEndCommandBuffer(_buffer);
    vk_check(result);
}

VkCommandBufferSubmitInfo vk::CommandBuffer::submit_info()
{
    VkCommandBufferSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

    info.deviceMask = 0;
    info.commandBuffer = _buffer;

    return info;
}