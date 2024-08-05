#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>

#include "vk/vulkan_context.h"
#include "vk/pipeline_builder.h"
#include "vk/vulkan_error.h"

Window::Window(int width, int height, std::string_view title) : width(width), height(height), title(title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);

    this->context.emplace("ugo-vk", *this);
}

// If we need to later, move this to a method on a CommandPool class.
VkCommandBuffer create_command_buffer(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    info.commandPool = pool;
    info.commandBufferCount = 1;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkCommandBuffer buffer;
    auto result = vkAllocateCommandBuffers(device, &info, &buffer);
    vk_check(result);

    return buffer;
}

VkFence create_fence(VkDevice device, bool start_signaled)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (start_signaled)
    {
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VkFence fence;
    auto result = vkCreateFence(device, &info, nullptr, &fence);
    vk_check(result);

    return fence;
}

VkSemaphore create_semaphore(VkDevice device, VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    info.flags = flags;
    
    VkSemaphore semaphore;
    auto result = vkCreateSemaphore(device, &info, nullptr, &semaphore);
    vk_check(result);

    return semaphore;
}

void begin_command_buffer(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    info.flags = flags;

    auto result = vkBeginCommandBuffer(buffer, &info);
    vk_check(result);
}

VkImageSubresourceRange get_image_range(VkImageAspectFlags flags)
{
    VkImageSubresourceRange subresource = {};
    // Update this when we support depth buffers.
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;;
    subresource.baseArrayLayer = 0;
    subresource.baseMipLevel = 0;
    subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource.levelCount = VK_REMAINING_MIP_LEVELS;
    
    return subresource;
}

void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout next_layout)
{
    VkImageMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    barrier.oldLayout = current_layout;
    barrier.newLayout = next_layout;

    barrier.subresourceRange = get_image_range(VK_IMAGE_ASPECT_COLOR_BIT);

    barrier.image = image;

    VkDependencyInfo dep_info = {};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep_info);
}

VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

    info.semaphore = semaphore;
    info.stageMask = stage_mask;

    info.deviceIndex = 0;
    info.value = 1;

    return info;
}

VkCommandBufferSubmitInfo command_submit_info(VkCommandBuffer cmd)
{
    VkCommandBufferSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

    info.deviceMask = 0;
    info.commandBuffer = cmd;

    return info;
}

VkSubmitInfo2 create_submit_info(VkCommandBufferSubmitInfo* buffer_submit, VkSemaphoreSubmitInfo* wait, VkSemaphoreSubmitInfo* signal)
{
    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = buffer_submit;

    info.waitSemaphoreInfoCount = 1;
    info.pWaitSemaphoreInfos = wait;

    info.signalSemaphoreInfoCount = 1;
    info.pSignalSemaphoreInfos = signal;

    return info;
}

const uint64_t ONE_SEC_NS = 1000000000;

void Window::run()
{
    VulkanDevice& device = this->context.value().device();

    PipelineBuilder builder(device);
    builder.set_vertex_shader_from_file("shader/tri.vert.spv");
    builder.set_fragment_shader_from_file("shader/tri.frag.spv");
    builder.set_color_format(this->context.value().swapchain().surface_format());
    builder.set_depth_format(VK_FORMAT_UNDEFINED);

    GraphicsPipeline pipeline = builder.build();

    VkCommandPool command_pool = device.alloc_graphics_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer command_buffer = create_command_buffer(device.device(), command_pool);

    VkFence render_fence = create_fence(device.device(), true);
    VkSemaphore acquire_semaphore = create_semaphore(device.device(), 0);
    VkSemaphore render_semaphore = create_semaphore(device.device(), 0);

    uint64_t frame_idx = 0;

    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();

        // Wait for last frame to be finished.
        auto result = vkWaitForFences(device.device(), 1, &render_fence, VK_TRUE, ONE_SEC_NS);
        vk_check(result);

        // And then reset our fence for the next frame.
        result = vkResetFences(device.device(), 1, &render_fence);
        vk_check(result);

        result = vkResetCommandBuffer(command_buffer, 0);
        vk_check(result);

        begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        uint32_t swap_image_idx = this->context.value().swapchain().acquire_image(acquire_semaphore);
        VkImage swap_image = this->context.value().swapchain().get_swapchain_image(swap_image_idx);

        transition_image(command_buffer, swap_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clear_color;
        clear_color = { {1.0f, (float)std::abs(std::sin((double)frame_idx / 10)), 1.0f, 1.0f} };
        VkImageSubresourceRange clear_range = get_image_range(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(command_buffer, swap_image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);

        transition_image(command_buffer, swap_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        result = vkEndCommandBuffer(command_buffer);
        vk_check(result);

        VkSemaphoreSubmitInfo wait_submit = semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, acquire_semaphore);
        VkSemaphoreSubmitInfo signal_submit = semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, render_semaphore);
        VkCommandBufferSubmitInfo buffer_submit_info = command_submit_info(command_buffer);

        VkSubmitInfo2 submit_info = create_submit_info(&buffer_submit_info, &wait_submit, &signal_submit);

        result = vkQueueSubmit2(device.graphics_queue(), 1, &submit_info, render_fence);
        vk_check(result);

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.swapchainCount = 1;
        VkSwapchainKHR swapchain = this->context.value().swapchain().swapchain();
        present_info.pSwapchains = &swapchain;
        
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_semaphore;

        present_info.pImageIndices = &swap_image_idx;

        result = vkQueuePresentKHR(device.graphics_queue(), &present_info);
        vk_check(result);

        frame_idx++;
    }

    vkDestroyCommandPool(device.device(), command_pool, nullptr);
    vkDestroyPipelineLayout(device.device(), pipeline.layout, nullptr);
    vkDestroyPipeline(device.device(), pipeline.pipeline, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
}