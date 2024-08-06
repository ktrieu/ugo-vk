#pragma once

#include <string>
#include <optional>

#include "vk/context.h"

struct GLFWwindow;

class Window
{
public:
    Window(int width, int height, std::string_view title);
    ~Window();

    GLFWwindow* get_window() { return this->window; }

    void run();

private:
    GLFWwindow *window = nullptr;
    std::optional<vk::Context> context;

    int width;
    int height;
    std::string title;
};