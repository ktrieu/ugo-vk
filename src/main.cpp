#include <stdexcept>

#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "window/window.h"
#include "vk/vulkan_context.h"
#include "logger.h"

int main(int argc, char **argv)
{
    int result = glfwInit();
    if (result == GLFW_FALSE)
    {
        log_glfw_error();
        return 1;
    }

    try
    {
        Logger::initialize();
        Window window(1080, 720, "ugo-vk");
        VulkanContext vulkan_context("ugo-vk", window);

        window.run();
    }
    catch (std::runtime_error &e)
    {
        log("Runtime error: {}", e.what());
    }

    glfwTerminate();

    return 0;
}