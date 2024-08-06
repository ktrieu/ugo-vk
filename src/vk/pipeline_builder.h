#pragma once

#include <string_view>

#include <vulkan/vulkan.h>

namespace vk {

	class Device;

	struct GraphicsPipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	class PipelineBuilder {
	public:
		PipelineBuilder(vk::Device& device);

		GraphicsPipeline build();

		void set_vertex_shader_from_file(std::string_view filename);
		void set_fragment_shader_from_file(std::string_view filename);

		void set_color_format(VkFormat format);
		void set_depth_format(VkFormat format);

	private:
		vk::Device& _device;

		VkShaderModule _vertex_shader = VK_NULL_HANDLE;
		VkShaderModule _fragment_shader = VK_NULL_HANDLE;

		VkFormat _color_format = VK_FORMAT_UNDEFINED;
		VkFormat _depth_format = VK_FORMAT_UNDEFINED;
	};
}