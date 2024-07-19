#include "vulkan_context.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <GLFW/glfw3.h>

VulkanContext::VulkanContext(std::string_view app_name) : app_name(app_name)
{
    this->create_instance();
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
            std::cout << "No debug messenger destroy function found.\n";
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
            std::string err;
            err.append("Required validation layer ");
            err.append(required_layer);
            err.append(" not found.");
            throw std::runtime_error(err);
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    std::cerr << "Validation layer message: " << pCallbackData->pMessage << std::endl;

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
