#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>

#include "vk/context.h"
#include "vk/pipeline_builder.h"
#include "vk/command_buffer.h"
#include "vk/vulkan_error.h"
#include "vk/sync.h"

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
    vk::Device& device = this->context.value().device();

    vk::PipelineBuilder builder(device);
    builder.set_vertex_shader_from_file("shader/tri.vert.spv");
    builder.set_fragment_shader_from_file("shader/tri.frag.spv");
    builder.set_color_format(this->context.value().swapchain().surface_format());
    builder.set_depth_format(VK_FORMAT_UNDEFINED);

    vk::GraphicsPipeline pipeline = builder.build();

    VkCommandPool command_pool = device.alloc_graphics_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk::CommandBuffer cmd(device.device(), command_pool);

    vk::Fence render_fence(device, VK_FENCE_CREATE_SIGNALED_BIT);
    vk::Semaphore swap_acquired(device, 0);
    vk::Semaphore render_complete(device, 0);

    uint64_t frame_idx = 0;

    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();

        render_fence.wait(ONE_SEC_NS);
        render_fence.reset();

        cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        uint32_t swap_image_idx = this->context.value().swapchain().acquire_image(swap_acquired);
        VkImage swap_image = this->context.value().swapchain().get_swapchain_image(swap_image_idx);

        transition_image(cmd.buffer(), swap_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clear_color;
        clear_color = { {1.0f, (float)std::abs(std::sin((double)frame_idx / 10)), 1.0f, 1.0f} };
        VkImageSubresourceRange clear_range = get_image_range(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(cmd.buffer(), swap_image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);

        transition_image(cmd.buffer(), swap_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        cmd.end();

        VkSemaphoreSubmitInfo wait_submit = swap_acquired.submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
        VkSemaphoreSubmitInfo signal_submit = render_complete.submit_info(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
        VkCommandBufferSubmitInfo buffer_submit_info = cmd.submit_info();

        VkSubmitInfo2 submit_info = create_submit_info(&buffer_submit_info, &wait_submit, &signal_submit);

        auto result = vkQueueSubmit2(device.graphics_queue(), 1, &submit_info, render_fence.vk_fence());
        vk_check(result);

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.swapchainCount = 1;
        VkSwapchainKHR swapchain = this->context.value().vk_swapchain();
        present_info.pSwapchains = &swapchain;
        
        present_info.waitSemaphoreCount = 1;
        VkSemaphore present_wait = render_complete.vk_semaphore();
        present_info.pWaitSemaphores = &present_wait;

        present_info.pImageIndices = &swap_image_idx;

        result = vkQueuePresentKHR(device.graphics_queue(), &present_info);
        vk_check(result);

        frame_idx++;
    }

    auto result = vkDeviceWaitIdle(device.device());
    vk_check(result);

    vkDestroyCommandPool(device.device(), command_pool, nullptr);
    vkDestroyPipelineLayout(device.device(), pipeline.layout, nullptr);
    vkDestroyPipeline(device.device(), pipeline.pipeline, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
}