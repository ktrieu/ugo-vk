#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

void Window::run()
{
    PipelineBuilder builder(this->context.value().get_device());
    builder.set_vertex_shader_from_file("shader/tri.vert.spv");
    builder.set_fragment_shader_from_file("shader/tri.frag.spv");
    builder.set_color_format(this->context.value().get_swapchain().get_surface_format());
    builder.set_depth_format(VK_FORMAT_UNDEFINED);

    GraphicsPipeline pipeline = builder.build();

    VkCommandPool command_pool = this->context.value().get_device().alloc_graphics_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer command_buffer = create_command_buffer(this->context.value().get_device().get_device(), command_pool);

    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }

    vkDestroyCommandPool(this->context.value().get_device().get_device(), command_pool, nullptr);
    vkDestroyPipelineLayout(this->context.value().get_device().get_device(), pipeline.layout, nullptr);
    vkDestroyPipeline(this->context.value().get_device().get_device(), pipeline.pipeline, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
}