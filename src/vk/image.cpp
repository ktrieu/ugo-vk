#include "image.h"

VkImageSubresourceRange vk::get_image_range(VkImageAspectFlags aspect) {
    VkImageSubresourceRange subresource = {};

    subresource.aspectMask = aspect;
    subresource.baseArrayLayer = 0;
    subresource.baseMipLevel = 0;
    subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource.levelCount = VK_REMAINING_MIP_LEVELS;

    return subresource;
}

void vk::transition_image(VkCommandBuffer cmd, VkImage image, VkImageSubresourceRange range, ImageBarrierState old_state, ImageBarrierState new_state) {
    VkImageMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

    barrier.srcStageMask = old_state.stage;
    barrier.srcAccessMask = old_state.access;
    barrier.oldLayout = old_state.layout;

    barrier.dstStageMask = new_state.stage;
    barrier.dstAccessMask = new_state.access;
    barrier.newLayout = new_state.layout;

    barrier.subresourceRange = range;

    barrier.image = image;

    VkDependencyInfo dep_info = {};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep_info);
}