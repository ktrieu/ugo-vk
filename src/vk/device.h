#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <optional>

#include "physical_device.h"


namespace vk {
    class Context;

    class Device {
    public:
        Device(vk::Context &context, PhysicalDevice &device_info);
        void destroy();

        PhysicalDevice &physical_device() { return this->_physical_device; }
        VkDevice device() { return this->_device; }

        uint32_t graphics_family() { return this->_graphics_family; }
        uint32_t transfer_family() { return this->_transfer_family; }
        uint32_t present_family() { return this->_present_family; }

        VkQueue graphics_queue() { return this->_graphics_queue; }
        VkQueue transfer_queue() { return this->_transfer_queue; }
        VkQueue present_queue() { return this->_present_queue; }

        VkCommandPool alloc_graphics_pool(VkCommandPoolCreateFlags flags);
        VkCommandPool alloc_transfer_pool(VkCommandPoolCreateFlags flags);

    private:
        void create_logical_device();

        vk::Context& _context;
        VkDevice _device;
        PhysicalDevice _physical_device;

        uint32_t _graphics_family;
        VkQueue _graphics_queue;

        uint32_t _transfer_family;
        VkQueue _transfer_queue;

        uint32_t _present_family;
        VkQueue _present_queue;
    };
}
