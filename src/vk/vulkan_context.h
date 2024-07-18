#include "vulkan/vulkan.h"

#include <string>
#include <vector>

class VulkanContext
{
public:
    VulkanContext(std::string_view app_name);

private:
    std::vector<const char *> get_required_extensions();
    void create_instance();

    VkInstance instance;
    std::string app_name;
};