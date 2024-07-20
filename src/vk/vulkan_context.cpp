#include "vulkan_context.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <fmt/core.h>

VulkanContext::VulkanContext(std::string_view app_name) : app_name(app_name)
{
    this->create_instance();
    this->device.emplace(this->select_physical_device());
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
    vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

    std::vector<VkLayerProperties> available_layers(num_layers);
    vkEnumerateInstanceLayerProperties(&num_layers, available_layers.data());

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

    VkResult create_result = vkCreateInstance(&create_info, nullptr, &this->instance);
    if (create_result != VK_SUCCESS)
    {
        throw std::runtime_error("Could not initialize Vulkan.");
    }

    if (this->enable_validation_layers)
    {
        this->create_debug_messenger();
    }
}

VulkanDevice VulkanContext::select_physical_device()
{
    uint32_t num_available;
    vkEnumeratePhysicalDevices(this->instance, &num_available, nullptr);

    std::vector<VkPhysicalDevice> available(num_available);
    vkEnumeratePhysicalDevices(this->instance, &num_available, available.data());

    std::vector<int> device_scores(num_available);

    for (int i = 0; i < num_available; i++)
    {
        VkPhysicalDeviceProperties2 properties = {};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(available[i], &properties);
        fmt::println("Device {}: {}", i, properties.properties.deviceName);
    }

    // Just return the first one for now.
    return VulkanDevice(*this, available[0]);
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
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create debug messenger.");
    }
}
