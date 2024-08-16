#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>

#include "vk/context.h"
#include "vk/pipeline_builder.h"
#include "vk/command_buffer.h"
#include "vk/vulkan_error.h"
#include "vk/sync.h"
#include "vk/image.h"

Window::Window(int width, int height, std::string_view title) : width(width), height(height), title(title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);

    this->context.emplace("ugo-vk", *this);
}

VkImageSubresourceRange get_image_range(VkImageAspectFlags flags)
{
    VkImageSubresourceRange subresource = {};
    // Update this when we support depth buffers.
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseArrayLayer = 0;
    subresource.baseMipLevel = 0;
    subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource.levelCount = VK_REMAINING_MIP_LEVELS;
    
    return subresource;
}

VkRenderingAttachmentInfo create_color_attachment_info(VkImageView view, std::optional<VkClearValue> clear, VkImageLayout layout)
{
    VkRenderingAttachmentInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;

    info.imageView = view;
    info.imageLayout = layout;

    if (clear.has_value()) {
        info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.clearValue = clear.value();
    }
    else {
        info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    }

    info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

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

    vk::ImageBarrierState swapchain_image_state = {};
    swapchain_image_state.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    swapchain_image_state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapchain_image_state.access = VK_ACCESS_2_MEMORY_WRITE_BIT;

    vk::ImageBarrierState render_image_state = {};
    render_image_state.stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    render_image_state.layout = VK_IMAGE_LAYOUT_GENERAL;
    render_image_state.access = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    vk::ImageBarrierState present_image_state = {};
    present_image_state.stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    present_image_state.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    present_image_state.access = 0;

    VkImageSubresourceRange image_range = vk::get_image_range(VK_IMAGE_ASPECT_COLOR_BIT);

    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();

        render_fence.wait(ONE_SEC_NS);
        render_fence.reset();

        cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        uint32_t swap_image_idx = this->context.value().swapchain().acquire_image(swap_acquired);
        VkImage swap_image = this->context.value().swapchain().get_swapchain_image(swap_image_idx);
        VkImageView swap_image_view = this->context.value().swapchain().get_swapchain_image_view(swap_image_idx);

        vk::transition_image(cmd.buffer(), swap_image, image_range, swapchain_image_state, render_image_state);

        VkClearColorValue clear_color;
        clear_color = { {1.0f, (float)std::abs(std::sin((double)frame_idx / 10)), 1.0f, 1.0f} };
        
        VkRenderingAttachmentInfo color_attachment_info = create_color_attachment_info(swap_image_view, VkClearValue { clear_color }, render_image_state.layout);
        VkRenderingInfo rendering_info = {};
        rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        
        rendering_info.colorAttachmentCount = 1;
        rendering_info.pColorAttachments = &color_attachment_info;
        rendering_info.layerCount = 1;
        rendering_info.renderArea.extent = this->context.value().swapchain().get_swap_extent();
        rendering_info.renderArea.offset = { 0, 0 };

        vkCmdBeginRendering(cmd.buffer(), &rendering_info);

        vkCmdBindPipeline(cmd.buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = this->width;
        viewport.height = this->height;
        viewport.maxDepth = 1.0f;
        viewport.minDepth = 0.0f;
        vkCmdSetViewport(cmd.buffer(), 0, 1, &viewport);

        VkRect2D scissor = rendering_info.renderArea;
        vkCmdSetScissor(cmd.buffer(), 0, 1, &scissor);

        vkCmdDraw(cmd.buffer(), 3, 1, 0, 0);

        vkCmdEndRendering(cmd.buffer());

        vk::transition_image(cmd.buffer(), swap_image, image_range, render_image_state, present_image_state);

        cmd.end();

        VkSemaphoreSubmitInfo wait_submit = swap_acquired.submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
        VkSemaphoreSubmitInfo signal_submit = render_complete.submit_info(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
        VkCommandBufferSubmitInfo buffer_submit_info = cmd.submit_info();

        VkSubmitInfo2 submit_info = create_submit_info(&buffer_submit_info, &wait_submit, &signal_submit);

        auto result = vkQueueSubmit2(device.graphics_queue(), 1, &submit_info, render_fence.vk_fence());
        vk_check(result);

        this->context.value().swapchain().present(swap_image_idx, device.graphics_queue(), render_complete);

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