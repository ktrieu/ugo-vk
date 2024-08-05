#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <optional>

#include "vulkan_device.h"
#include "swapchain.h"

class Window;

class VulkanContext
{
public:
    VulkanContext(std::string_view app_name, Window &window);
    ~VulkanContext();

    VulkanDevice& device() { return _device.value(); }
    Swapchain& swapchain() { return this->_swapchain.value(); }
    VkSurfaceKHR surface() { return this->_surface; }

private:
    std::vector<const char *> get_required_extensions();
    std::vector<const char *> get_validation_layers();
    void create_instance();

    void create_surface(Window &window);

    VulkanDevice select_physical_device();

    void create_debug_messenger();

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;

    VkSurfaceKHR _surface;

    std::optional<Swapchain> _swapchain;
    std::optional<VulkanDevice> _device;

    std::string _app_name;
};