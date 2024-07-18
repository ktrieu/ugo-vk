#include "vulkan_context.h"

#include <string>
#include <stdexcept>

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

    // Just return the GLFW extension list for now - we may want more later.
    return std::vector<const char *>(glfwExtensions, glfwExtensions + numGlfwExtensions);
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

    VkResult create_result = vkCreateInstance(&create_info, nullptr, &this->instance);
    if (create_result != VK_SUCCESS)
    {
        throw std::runtime_error("Could not initialize Vulkan.");
    }
}