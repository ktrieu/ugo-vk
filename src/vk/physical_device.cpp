#include "physical_device.h"

#include "logger.h"
#include "vulkan_error.h"

const std::vector<const char *> PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
#if defined(__APPLE__) && defined(__MACH__)
	"VK_KHR_portability_subset",
#endif
};

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface) : device(device)
{
	this->properties = {};
	this->properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	vkGetPhysicalDeviceProperties2(device, &this->properties);

	this->features = {};
	this->features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	vkGetPhysicalDeviceFeatures2(device, &this->features);

	uint32_t num_queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties2(device, &num_queue_families, nullptr);

	VkQueueFamilyProperties2 default_queue_family = {};
	default_queue_family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
	this->queue_families.resize(num_queue_families, default_queue_family);
	vkGetPhysicalDeviceQueueFamilyProperties2(device, &num_queue_families, this->queue_families.data());

	uint32_t num_extensions;
	auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, nullptr);
	vk_check(result);

	this->extensions.resize(num_extensions);
	result = vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, this->extensions.data());
	vk_check(result);

	VkPhysicalDeviceSurfaceInfo2KHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
	surface_info.surface = surface;

	this->surface_caps = {};
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->device, surface, &this->surface_caps);
	vk_check(result);

	uint32_t num_formats;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(this->device, surface, &num_formats, nullptr);
	vk_check(result);

	this->surface_formats.resize(num_formats);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(this->device, surface, &num_formats, this->surface_formats.data());
	vk_check(result);

	uint32_t num_modes;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(this->device, surface, &num_modes, nullptr);
	vk_check(result);

	this->present_modes.resize(num_modes);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(this->device, surface, &num_modes, this->present_modes.data());

	this->graphics_families = get_queue_families_for_type(VK_QUEUE_GRAPHICS_BIT);
	this->transfer_families = get_queue_families_for_type(VK_QUEUE_TRANSFER_BIT);

	this->present_families = get_present_families(surface);
}

bool PhysicalDevice::is_usable()
{
	if (!this->get_graphics_family().has_value())
	{
		return false;
	}

	if (!this->get_present_family().has_value())
	{
		return false;
	}

	if (this->surface_formats.size() == 0 || this->present_modes.size() == 0)
	{
		return false;
	}

	for (auto required_ext : PhysicalDevice::REQUIRED_DEVICE_EXTENSIONS)
	{
		auto result = std::find_if(this->extensions.begin(), this->extensions.end(), [required_ext](VkExtensionProperties ext)
								   { return std::strcmp(ext.extensionName, required_ext) == 0; });

		if (result == this->extensions.end())
		{
			log("Extension {} not found.", required_ext);
			return false;
		}
	}

	return true;
}

std::string_view PhysicalDevice::get_name()
{
	return this->properties.properties.deviceName;
}

std::optional<uint32_t> PhysicalDevice::get_graphics_family()
{
	if (this->graphics_families.size() == 0)
	{
		return std::nullopt;
	}

	return this->graphics_families[0];
}

std::optional<uint32_t> PhysicalDevice::get_transfer_family()
{
	// We want a family that isn't the same as the graphics family, preferably.
	auto graphics_family = this->get_graphics_family();

	for (auto &idx : this->transfer_families)
	{
		if (graphics_family.has_value() && idx != graphics_family.value())
		{
			return idx;
		}
	}

	// If we can't, graphics queues can always accept transfer commands anyway.
	return graphics_family;
}

std::optional<uint32_t> PhysicalDevice::get_present_family()
{
	if (this->get_graphics_family().has_value())
	{
		// We'd like the present and graphics family to be the same.
		auto result = std::find(this->present_families.begin(), this->present_families.end(), this->get_graphics_family().value());
		if (result != this->present_families.end())
		{
			return *result;
		}
	}

	// But if not, just return whatever we have.
	if (this->present_families.size() != 0)
	{
		return this->present_families[0];
	}

	return std::nullopt;
}

std::vector<uint32_t> PhysicalDevice::get_queue_families_for_type(VkQueueFlags ty)
{
	std::vector<uint32_t> families;
	for (int i = 0; i < this->queue_families.size(); i++)
	{
		if ((this->queue_families[i].queueFamilyProperties.queueFlags & ty) != 0)
		{
			families.push_back(i);
		}
	}

	return families;
}

std::vector<uint32_t> PhysicalDevice::get_present_families(VkSurfaceKHR surface)
{
	std::vector<uint32_t> families;
	for (int i = 0; i < this->queue_families.size(); i++)
	{
		VkBool32 supported;
		auto result = vkGetPhysicalDeviceSurfaceSupportKHR(this->device, i, surface, &supported);
		vk_check(result);

		if (supported)
		{
			families.push_back(i);
		}
	}

	return families;
}
