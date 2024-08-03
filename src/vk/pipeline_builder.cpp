#include "pipeline_builder.h"

#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include <fmt/format.h>

#include "vk/vulkan_device.h"
#include "vk/vulkan_error.h"

PipelineBuilder::PipelineBuilder(VulkanDevice& device) : _device(device)
{

}

// Maybe move this to a class one day, idk.
VkShaderModule create_shader_module(VkDevice device, std::string_view filename)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	std::string filename_owned(filename);
	std::fstream file(filename_owned, std::ifstream::ate | std::ifstream::binary | std::ifstream::in);
	if (!file.good())
	{
		throw std::runtime_error(fmt::format("Could not open file {}.", filename));
	}

	size_t len = static_cast<size_t>(file.tellg());
	std::vector<char> data(len);
	file.seekg(0);
	file.read(data.data(), len);

	info.codeSize = len;
	info.pCode = (uint32_t*)data.data();

	VkShaderModule module;
	auto result = vkCreateShaderModule(device, &info, nullptr, &module);
	vk_check(result);

	return module;
}

void PipelineBuilder::set_vertex_shader_from_file(std::string_view filename)
{
	_vertex_shader = create_shader_module(this->_device.get_device(), filename);
}

void PipelineBuilder::set_fragment_shader_from_file(std::string_view filename)
{
	_fragment_shader = create_shader_module(this->_device.get_device(), filename);
}

VkPipelineShaderStageCreateInfo create_shader_stage_info(VkShaderModule shader, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	info.stage = stage;
	info.module = shader;
	info.pName = "main";

	return info;
}

void PipelineBuilder::set_color_format(VkFormat format)
{
	_color_format = format;
}

void PipelineBuilder::set_depth_format(VkFormat format)
{
	_depth_format = format;
}

VkPipeline PipelineBuilder::build()
{
	if (_vertex_shader == VK_NULL_HANDLE || _fragment_shader == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vertex and fragment shader must be set.");
	}

	if (_color_format == VK_FORMAT_UNDEFINED)
	{
		throw std::runtime_error("Color format must be set.");
	}

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	auto vertex_stage = create_shader_stage_info(_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
	auto fragment_stage = create_shader_stage_info(_fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipelineShaderStageCreateInfo stages[2] = {vertex_stage, fragment_stage};

	info.stageCount = 2;
	info.pStages = stages;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	info.pViewportState = &viewport_info;

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pVertexInputState = &vertex_input_info;

	VkPipelineColorBlendStateCreateInfo color_blend_info = {};
	color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_info.attachmentCount = 1;

	VkPipelineColorBlendAttachmentState blend_color_attachment = {};
	blend_color_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend_color_attachment.blendEnable = VK_FALSE;
	color_blend_info.pAttachments = &blend_color_attachment;

	info.pColorBlendState = &color_blend_info;

	VkPipelineInputAssemblyStateCreateInfo assembly_info = {};
	assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_info.primitiveRestartEnable = VK_FALSE;
	assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	info.pInputAssemblyState = &assembly_info;

	VkPipelineRasterizationStateCreateInfo raster_info = {};
	raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_info.cullMode = VK_CULL_MODE_NONE;
	raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	raster_info.polygonMode = VK_POLYGON_MODE_FILL;
	raster_info.lineWidth = 1.0f;
	info.pRasterizationState = &raster_info;

	VkPipelineMultisampleStateCreateInfo multi_info = {};
	multi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi_info.sampleShadingEnable = VK_FALSE;
	multi_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multi_info.minSampleShading = 1.0f;
	multi_info.alphaToCoverageEnable = VK_FALSE;
	multi_info.alphaToOneEnable = VK_FALSE;
	info.pMultisampleState = &multi_info;

	VkPipelineDepthStencilStateCreateInfo depth_info = {};
	depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	// Disable depth test for now. We'll probably want to turn this on at some later date.
	depth_info.depthTestEnable = VK_FALSE;
	depth_info.depthWriteEnable = VK_FALSE;
	depth_info.depthCompareOp = VK_COMPARE_OP_NEVER;
	depth_info.depthBoundsTestEnable = VK_FALSE;
	depth_info.stencilTestEnable = VK_FALSE;
	depth_info.front = {};
	depth_info.back = {};
	depth_info.minDepthBounds = 0.0f;
	depth_info.maxDepthBounds = 1.0f;
	info.pDepthStencilState = &depth_info;

	VkPipelineRenderingCreateInfo render_info = {};
	render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachmentFormats = &_color_format;
	render_info.depthAttachmentFormat = _depth_format;
	info.pNext = &render_info;

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.dynamicStateCount = 2;
	dynamic_info.pDynamicStates = dynamic_states;
	info.pDynamicState = &dynamic_info;

	VkPipelineLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// No descriptor sets or push constants yet, so don't initialize anything else.

	VkPipelineLayout layout;
	auto result = vkCreatePipelineLayout(_device.get_device(), &layout_info, nullptr, &layout);
	vk_check(result);
	info.layout = layout;

	VkPipeline pipeline;
	result = vkCreateGraphicsPipelines(_device.get_device(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
	vk_check(result);

	// We don't need the attached shaders anymore after the pipeline has been created.
	vkDestroyShaderModule(_device.get_device(), _vertex_shader, nullptr);
	vkDestroyShaderModule(_device.get_device(), _fragment_shader, nullptr);

	return pipeline;
}