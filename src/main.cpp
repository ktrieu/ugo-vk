#include <stdexcept>

#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "window/window.h"
#include "vk/vulkan_context.h"

int main(int argc, char **argv)
{
    int result = glfwInit();
    if (result == GLFW_FALSE)
    {
        const char *glfwError = nullptr;
        glfwGetError(&glfwError);
        fmt::println("GLFW initialization error: {}", glfwError);

        return 1;
    }

    try
    {
        Window window(1080, 720, "ugo-vk");
        VulkanContext vulkan_context("ugo-vk");

        window.run();
    }
    catch (std::runtime_error &e)
    {
        fmt::println("Runtime error: {}", e.what());
    }

    glfwTerminate();

    return 0;
}