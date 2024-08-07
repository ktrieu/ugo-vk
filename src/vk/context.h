#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <optional>

#include "device.h"
#include "swapchain.h"

class Window;

namespace vk {

    class Context {
    public:
        Context(std::string_view app_name, Window &window);
        Context(const Context& other) = delete;
        Context& operator=(const Context& other) = delete;

        ~Context();

        vk::Device& device() { return _device.value(); }
        VkDevice vk_device() { return _device.value().device(); }
        PhysicalDevice& physical_device() { return _device.value().physical_device(); }

        Swapchain& swapchain() { return this->_swapchain.value(); }
        VkSwapchainKHR vk_swapchain() { return this->_swapchain.value().swapchain(); }

        VkSurfaceKHR surface() { return this->_surface; }

    private:
        std::vector<const char *> get_required_extensions();
        std::vector<const char *> get_validation_layers();
        void create_instance();

        void create_surface(Window &window);

        vk::Device select_physical_device();

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
        std::optional<vk::Device> _device;

        std::string _app_name;
    };

}
