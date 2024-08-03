#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk/vulkan_context.h"
#include "vk/pipeline_builder.h"

Window::Window(int width, int height, std::string_view title) : width(width), height(height), title(title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);

    this->context.emplace("ugo-vk", *this);
}

void Window::run()
{
    PipelineBuilder builder(this->context.value().get_device());
    builder.set_vertex_shader_from_file("shader/tri.vert.spv");
    builder.set_fragment_shader_from_file("shader/tri.frag.spv");
    builder.set_color_format(this->context.value().get_swapchain().get_surface_format());
    builder.set_depth_format(VK_FORMAT_UNDEFINED);

    GraphicsPipeline pipeline = builder.build();

    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }

    vkDestroyPipelineLayout(this->context.value().get_device().get_device(), pipeline.layout, nullptr);
    vkDestroyPipeline(this->context.value().get_device().get_device(), pipeline.pipeline, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
}