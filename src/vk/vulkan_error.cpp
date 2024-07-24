#include "vulkan_error.h"

#include <fmt/format.h>

void vk_check(VkResult result)
{
	if (result == VK_SUCCESS)
	{
		return;
	}

	throw std::runtime_error(fmt::format("Vulkan error: {}", (int)result));
}
