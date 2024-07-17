#include <format>
#include <iostream>

#include <GLFW/glfw3.h>

#include "window/window.h"

int main(int argc, char **argv)
{
    int result = glfwInit();
    if (result == GLFW_FALSE)
    {
        const char *glfwError = nullptr;
        glfwGetError(&glfwError);
        std::cout << std::format("GLFW initalization error: {}\n", glfwError);

        return 1;
    }

    Window window(1080, 720, "ugo-vk");

    window.run();

    return 0;
}