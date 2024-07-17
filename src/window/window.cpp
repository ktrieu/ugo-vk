#include "window.h"

#include <GLFW/glfw3.h>

Window::Window(int width, int height, std::string_view title) : width(width), height(height), title(title)
{
    this->window = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);
}

void Window::run()
{
    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }
}