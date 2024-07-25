#pragma once

#include <string>

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

    int width;
    int height;
    std::string title;
};