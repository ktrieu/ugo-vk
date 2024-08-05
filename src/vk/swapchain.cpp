#include "swapchain.h"

#include <GLFW/glfw3.h>

#include <algorithm>

#include "vk/vulkan_context.h"
#include "vk/vulkan_error.h"
#include "window/window.h"

Swapchain::Swapchain(VulkanContext &context, Window &window) : _context(context)
{
    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = context.get_surface();

    auto surface_format = this->select_format();
    info.imageFormat = surface_format.format;
    info.imageColorSpace = surface_format.colorSpace;
    _surface_format = surface_format.format;

    info.presentMode = this->select_present_mode();
    info.clipped = VK_TRUE;

    _swap_extent = this->choose_swap_extent(window);
    info.imageExtent = _swap_extent;

    auto caps = _context.get_device().get_physical_device().get_surface_caps();
    // Request 1 more than the minimum so we don't wait on the driver.
    info.minImageCount = caps.minImageCount + 1;
    // Unless that's more than is supported. 0 means there is no maximum, so we can skip the check
    if (caps.maxImageCount != 0)
    {
        info.minImageCount = std::min(caps.minImageCount, caps.maxImageCount);
    }

    uint32_t graphics_family = _context.get_device().get_graphics_family();
    uint32_t present_family = _context.get_device().get_present_family();
    _image_sharing_required = graphics_family != present_family;
    uint32_t families[] = {graphics_family, present_family};

    if (_image_sharing_required)
    {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = families;
    }
    else
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    info.preTransform = caps.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.oldSwapchain = VK_NULL_HANDLE;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto result = vkCreateSwapchainKHR(_context.get_device().get_device(), &info, nullptr, &_swapchain);
    vk_check(result);

    uint32_t image_count;
    result = vkGetSwapchainImagesKHR(_context.get_device().get_device(), _swapchain, &image_count, nullptr);
    vk_check(result);

    _images.resize(image_count);
    result = vkGetSwapchainImagesKHR(_context.get_device().get_device(), _swapchain, &image_count, _images.data());
    vk_check(result);

    _image_views.resize(image_count);
    for (int i = 0; i < image_count; i++)
    {
        _image_views[i] = this->create_image_view(_images[i]);
    }
}

void Swapchain::destroy()
{
    for (auto view : _image_views)
    {
        vkDestroyImageView(_context.get_device().get_device(), view, nullptr);
    }

    vkDestroySwapchainKHR(_context.get_device().get_device(), _swapchain, nullptr);
}

uint32_t Swapchain::acquire_image(VkSemaphore completion)
{
    uint32_t image_idx;
    auto result = vkAcquireNextImageKHR(_context.get_device().get_device(), _swapchain, 1000000000, completion, nullptr, &image_idx);
    vk_check(result);

    return image_idx;
}

VkImage Swapchain::get_swapchain_image(uint32_t image_idx)
{
    return _images.at(image_idx);
}

VkSurfaceFormatKHR Swapchain::select_format()
{
    auto formats = _context.get_device().get_physical_device().get_surface_formats();

    // Pick a preferred format, or just default to the first one, whatever that is.
    for (auto &f : formats)
    {
        if (f.format == VK_FORMAT_B8G8R8_SRGB && f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return f;
        }
    }

    return formats[0];
}

VkPresentModeKHR Swapchain::select_present_mode()
{
    auto modes = _context.get_device().get_physical_device().get_present_modes();

    // We'd like mailbox mode...
    for (auto &m : modes)
    {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return m;
        }
    }

    // But FIFO is always available if mailbox isn't.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_swap_extent(Window &window)
{
    auto caps = _context.get_device().get_physical_device().get_surface_caps();

    // If the current extent's width is the max, we get to pick the swap extent.
    if (caps.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        int wnd_w, wnd_h;
        glfwGetFramebufferSize(window.get_window(), &wnd_w, &wnd_h);

        uint32_t clamped_w = std::clamp(caps.minImageExtent.width, (uint32_t)wnd_w, caps.maxImageExtent.width);
        uint32_t clamped_h = std::clamp(caps.minImageExtent.height, (uint32_t)wnd_h, caps.maxImageExtent.height);

        VkExtent2D extent;
        extent.width = clamped_w;
        extent.height = clamped_h;

        return extent;
    }

    // Otherwise, we just return whatever we're given.
    return caps.currentExtent;
}

VkImageView Swapchain::create_image_view(VkImage image)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = image;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = _surface_format;

    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;

    VkImageView view;
    auto result = vkCreateImageView(_context.get_device().get_device(), &info, nullptr, &view);
    vk_check(result);

    return view;
}