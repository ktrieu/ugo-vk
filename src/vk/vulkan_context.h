#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <optional>

#include "vulkan_device.h"
class VulkanContext
{
public:
    VulkanContext(std::string_view app_name);
    ~VulkanContext();

private:
    std::vector<const char *> get_required_extensions();
    std::vector<const char *> get_validation_layers();
    void create_instance();

    VulkanDevice select_physical_device();

    void create_debug_messenger();

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    std::optional<VulkanDevice> device;

    std::string app_name;
};