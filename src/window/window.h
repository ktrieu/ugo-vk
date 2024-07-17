#include <string>

struct GLFWwindow;

class Window
{
public:
    Window(int width, int height, std::string_view title);
    ~Window();
    void run();

private:
    GLFWwindow *window = nullptr;

    int width;
    int height;
    std::string title;
};