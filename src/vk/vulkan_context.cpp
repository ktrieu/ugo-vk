#include "vulkan_context.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "window/window.h"
#include "vulkan_error.h"
#include "logger.h"

VulkanContext::VulkanContext(std::string_view app_name, Window &window) : app_name(app_name)
{
    this->create_instance();
    this->create_surface(window);
    this->device.emplace(this->select_physical_device());
    this->swapchain.emplace(*this, window);
}

VulkanContext::~VulkanContext()
{
    if (this->enable_validation_layers)
    {
        auto debug_messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
        if (debug_messenger_destroy_func != nullptr)
        {
            debug_messenger_destroy_func(this->instance, this->debug_messenger, nullptr);
        }
        else
        {
            // Oops. Things are already broken if we can't find the destroy function. Let's just log and move on.
            fmt::println("No debug messenger destroy function found.");
        }
    }

    this->device.value().destroy();

    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);

    vkDestroyInstance(this->instance, nullptr);
}

std::vector<const char *> VulkanContext::get_required_extensions()
{
    uint32_t numGlfwExtensions;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&numGlfwExtensions);

    std::vector<const char *> required_extensions(glfwExtensions, glfwExtensions + numGlfwExtensions);

    if (this->enable_validation_layers)
    {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // We need this for MacOS, but it's OK to have for everyone.
    required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    // Just return the GLFW extension list for now - we may want more later.
    return required_extensions;
}

const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char *> VulkanContext::get_validation_layers()
{
    uint32_t num_layers;
    auto result = vkEnumerateInstanceLayerProperties(&num_layers, nullptr);
    vk_check(result);

    std::vector<VkLayerProperties> available_layers(num_layers);
    result = vkEnumerateInstanceLayerProperties(&num_layers, available_layers.data());
    vk_check(result);

    std::vector<const char *>
        validation_layers;

    for (const char *required_layer : REQUIRED_VALIDATION_LAYERS)
    {
        auto found_layer = std::find_if(
            available_layers.begin(),
            available_layers.end(),
            [required_layer](VkLayerProperties layer)
            {
                return std::strcmp(layer.layerName, required_layer) == 0;
            });

        if (found_layer != available_layers.end())
        {
            validation_layers.push_back(required_layer);
        }
        else
        {
            throw std::runtime_error(fmt::format("Required validation layer {} not found.", required_layer));
        }
    }

    return validation_layers;
}

void VulkanContext::create_instance()
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = this->app_name.c_str();
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto required_extensions = this->get_required_extensions();
    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    // We need this for MacOS, but it's OK to have for everyone.
    create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    auto validation_layers = this->get_validation_layers();
    if (this->enable_validation_layers)
    {
        create_info.enabledLayerCount = validation_layers.size();
        create_info.ppEnabledLayerNames = validation_layers.data();
    }

    VkResult result = vkCreateInstance(&create_info, nullptr, &this->instance);
    vk_check(result);

    if (this->enable_validation_layers)
    {
        this->create_debug_messenger();
    }
}

void VulkanContext::create_surface(Window &window)
{
    auto result = glfwCreateWindowSurface(this->instance, window.get_window(), nullptr, &this->surface);
    vk_check(result);
}

VulkanDevice VulkanContext::select_physical_device()
{
    uint32_t num_available;
    auto result = vkEnumeratePhysicalDevices(this->instance, &num_available, nullptr);
    vk_check(result);

    std::vector<VkPhysicalDevice> available(num_available);
    result = vkEnumeratePhysicalDevices(this->instance, &num_available, available.data());
    vk_check(result);

    std::vector<PhysicalDevice> device_infos;

    for (int i = 0; i < num_available; i++)
    {
        PhysicalDevice device_info(available[i], this->surface);

        log("Device {}: {}", i, device_info.get_name());
        if (device_info.is_usable())
        {
            device_infos.push_back(device_info);
        }
        else
        {
            log("Rejected device.");
        }
    }

    if (device_infos.size() == 0)
    {
        throw std::runtime_error("No usable physical devices found.");
    }

    // Just return the first one for now.
    log("Selected device {}: {}", 0, device_infos[0].get_name());
    return VulkanDevice(*this, device_infos[0]);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    fmt::println("Validation layer message: {}", pCallbackData->pMessage);

    return VK_FALSE;
}

void VulkanContext::create_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT info = {};

    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;

    auto create_func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
    if (create_func == nullptr)
    {
        throw std::runtime_error("Debug messenger create function not found.");
    }

    VkResult result = create_func(this->instance, &info, nullptr, &this->debug_messenger);
    vk_check(result);
}

VulkanDevice &VulkanContext::get_device()
{
    // This should never happen since we initialize the device in the constructor.
    if (!this->device.has_value())
    {
        throw std::runtime_error("Vulkan device not initialized!");
    }

    return this->device.value();
}