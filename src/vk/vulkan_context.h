#include "vulkan/vulkan.h"

#include <string>
#include <vector>

class VulkanContext
{
public:
    VulkanContext(std::string_view app_name);

private:
    std::vector<const char *> get_required_extensions();
    std::vector<const char *> get_validation_layers();
    void create_instance();

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    VkInstance instance;

    std::string app_name;
};