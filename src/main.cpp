#include <format>
#include <iostream>

#include <GLFW/glfw3.h>

int main(int argc, char **argv)
{
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(1080, 720, "ugo-vk", nullptr, nullptr);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    return 0;
}