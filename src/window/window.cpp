#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

Window::Window(int width, int height, std::string_view title) : width(width), height(height), title(title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);
}

void Window::run()
{
    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
}