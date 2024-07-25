#include "physical_device.h"

#include "logger.h"
#include "vulkan_error.h"

PhysicalDeviceInfo::PhysicalDeviceInfo(VkPhysicalDevice device, VkSurfaceKHR surface) : device(device)
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

	this->graphics_families = get_queue_families_for_type(VK_QUEUE_GRAPHICS_BIT);

	this->present_families = get_present_families(surface);
}

const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool PhysicalDeviceInfo::is_usable()
{
	if (!this->get_graphics_family().has_value())
	{
		return false;
	}

	if (!this->get_present_family().has_value())
	{
		return false;
	}

	for (auto required_ext : REQUIRED_DEVICE_EXTENSIONS)
	{
		auto result = std::find_if(this->extensions.begin(), this->extensions.end(), [required_ext](VkExtensionProperties ext) {
			return std::strcmp(ext.extensionName, required_ext) == 0;
			});

		if (result == this->extensions.end())
		{
			log("Extension {} not found.", required_ext);
			return false;
		}
	}

	return true;
}

std::string_view PhysicalDeviceInfo::get_name()
{
	return this->properties.properties.deviceName;
}

std::optional<uint32_t> PhysicalDeviceInfo::get_graphics_family()
{
	if (this->graphics_families.size() == 0)
	{
		return std::nullopt;
	}

	return this->graphics_families[0];
}

std::optional<uint32_t> PhysicalDeviceInfo::get_present_family()
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

std::vector<uint32_t> PhysicalDeviceInfo::get_queue_families_for_type(VkQueueFlags ty)
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

std::vector<uint32_t> PhysicalDeviceInfo::get_present_families(VkSurfaceKHR surface)
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
