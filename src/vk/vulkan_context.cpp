#include "vulkan_context.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <format>

#include <GLFW/glfw3.h>

VulkanContext::VulkanContext(std::string_view app_name) : app_name(app_name)
{
    this->create_instance();
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
            throw std::runtime_error(std::format("Required validation layer {} not found.", required_layer));
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
}