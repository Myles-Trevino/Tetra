#include <array>
#include "../Oreginum//Core.hpp"
#include "Pipeline.hpp"

Oreginum::Vulkan::Pipeline::Pipeline(std::shared_ptr<Device> device, const glm::uvec2& resolution,
	const Render_Pass& render_pass, const Shader& shader,
	std::vector<vk::DescriptorSetLayout> descriptor_set_layouts, 
	uint8_t render_pass_number, const Pipeline& base)
	: device(device), descriptor_set_layouts(descriptor_set_layouts)
{
	//Vertex input
	std::vector<vk::VertexInputBindingDescription> binding_descriptions;
	std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;

	if(render_pass_number == 0)
	{
		binding_descriptions.resize(1);
		binding_descriptions[0].setBinding(0);
		binding_descriptions[0].setStride(sizeof(float)*9);
		binding_descriptions[0].setInputRate(vk::VertexInputRate::eVertex);

		attribute_descriptions.resize(4);

		//Vertex
		attribute_descriptions[0].setBinding(0);
		attribute_descriptions[0].setLocation(0);
		attribute_descriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
		attribute_descriptions[0].setOffset(0);

		//UVs
		attribute_descriptions[1].setBinding(0);
		attribute_descriptions[1].setLocation(1);
		attribute_descriptions[1].setFormat(vk::Format::eR32G32Sfloat);
		attribute_descriptions[1].setOffset(sizeof(float)*3);

		//Normals
		attribute_descriptions[2].setBinding(0);
		attribute_descriptions[2].setLocation(2);
		attribute_descriptions[2].setFormat(vk::Format::eR32G32B32Sfloat);
		attribute_descriptions[2].setOffset(sizeof(float)*5);

		//Voxel material
		attribute_descriptions[3].setBinding(0);
		attribute_descriptions[3].setLocation(3);
		attribute_descriptions[3].setFormat(vk::Format::eR32Sfloat);
		attribute_descriptions[3].setOffset(sizeof(float)*8);
	}

	if(render_pass_number == 1 || render_pass_number == 2)
	{
		binding_descriptions.resize(1);
		binding_descriptions[0].setBinding(0);
		binding_descriptions[0].setStride(sizeof(float)*9);
		binding_descriptions[0].setInputRate(vk::VertexInputRate::eVertex);

		attribute_descriptions.resize(1);

		//Vertex
		attribute_descriptions[0].setBinding(0);
		attribute_descriptions[0].setLocation(0);
		attribute_descriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
		attribute_descriptions[0].setOffset(0);
	}

	vk::PipelineVertexInputStateCreateInfo vertex_input_state_information;
	vertex_input_state_information.setVertexBindingDescriptionCount(
		static_cast<uint32_t>(binding_descriptions.size()));
	vertex_input_state_information.setPVertexBindingDescriptions(binding_descriptions.data());
	vertex_input_state_information.setVertexAttributeDescriptionCount(
		static_cast<uint32_t>(attribute_descriptions.size()));
	vertex_input_state_information.setPVertexAttributeDescriptions(
		attribute_descriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_information;
	input_assembly_state_information.setTopology(vk::PrimitiveTopology::eTriangleList);
	input_assembly_state_information.setPrimitiveRestartEnable(VK_FALSE);

	//Viewport
	vk::Viewport viewport;
	viewport.setX(0);
	viewport.setY(0);
	viewport.setWidth(static_cast<float>(resolution.x));
	viewport.setHeight(static_cast<float>(resolution.y));
	viewport.setMinDepth(0.f);
	viewport.setMaxDepth(1.f);

	vk::Rect2D scissor;
	scissor.setOffset({0, 0});
	scissor.setExtent(vk::Extent2D{resolution.x, resolution.y});

	vk::PipelineViewportStateCreateInfo viewport_state_information;
	viewport_state_information.setViewportCount(1);
	viewport_state_information.setPViewports(&viewport);
	viewport_state_information.setScissorCount(1);
	viewport_state_information.setPScissors(&scissor);

	//Rasterization
	vk::PipelineRasterizationStateCreateInfo rasterization_state_information;
	rasterization_state_information.setDepthClampEnable(VK_FALSE);
	rasterization_state_information.setRasterizerDiscardEnable(VK_FALSE);
	rasterization_state_information.setPolygonMode(vk::PolygonMode::eFill);
	rasterization_state_information.setCullMode(vk::CullModeFlagBits::eBack);
	rasterization_state_information.setFrontFace(render_pass_number >= 3
		? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise);
	rasterization_state_information.setDepthBiasEnable(VK_FALSE);
	rasterization_state_information.setDepthBiasConstantFactor(0);
	rasterization_state_information.setDepthBiasClamp(0);
	rasterization_state_information.setDepthBiasSlopeFactor(0);
	rasterization_state_information.setLineWidth(1);

	//Multisampling
	vk::PipelineMultisampleStateCreateInfo multisample_state_information;
	multisample_state_information.setRasterizationSamples(
		render_pass_number == 0 || render_pass_number == 2 ||
		render_pass_number == 5 ? Swapchain::SAMPLES : vk::SampleCountFlagBits::e1);
	multisample_state_information.setSampleShadingEnable(false);
	multisample_state_information.setMinSampleShading(0);
	multisample_state_information.setPSampleMask(nullptr);
	multisample_state_information.setAlphaToCoverageEnable(false);
	multisample_state_information.setAlphaToOneEnable(false);

	//Blending
	vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
	color_blend_attachment_state.setBlendEnable(false);
	color_blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eR | 
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | 
		vk::ColorComponentFlagBits::eA);
	color_blend_attachment_state.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
	color_blend_attachment_state.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
	color_blend_attachment_state.setColorBlendOp(vk::BlendOp::eAdd);
	color_blend_attachment_state.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	color_blend_attachment_state.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	color_blend_attachment_state.setAlphaBlendOp(vk::BlendOp::eAdd);

	std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachments;
	if(render_pass_number == 0) color_blend_attachments = {color_blend_attachment_state,
		color_blend_attachment_state, color_blend_attachment_state, color_blend_attachment_state};
	else if(render_pass_number != 1) color_blend_attachments = {color_blend_attachment_state};

	vk::PipelineColorBlendStateCreateInfo color_blend_state_information;
	color_blend_state_information.setLogicOpEnable(VK_FALSE);
	color_blend_state_information.setLogicOp(vk::LogicOp::eCopy);
	color_blend_state_information.setAttachmentCount(
		static_cast<uint32_t>(color_blend_attachments.size()));
	color_blend_state_information.setPAttachments(color_blend_attachments.data());
	color_blend_state_information.setBlendConstants({0, 0, 0, 0});

	//Depth stencil
	vk::PipelineDepthStencilStateCreateInfo depth_stencil_information;
	if(render_pass_number < 3)
	{
		depth_stencil_information.setDepthTestEnable(VK_TRUE);
		depth_stencil_information.setDepthWriteEnable(VK_TRUE);
		depth_stencil_information.setDepthCompareOp(vk::CompareOp::eLess);
	}

	//Layout
	vk::PipelineLayoutCreateInfo layout_information;
	layout_information.setSetLayoutCount(static_cast<uint32_t>(descriptor_set_layouts.size()));
	layout_information.setPSetLayouts(descriptor_set_layouts.data());
	layout_information.setPushConstantRangeCount(0);
	layout_information.setPPushConstantRanges(nullptr);

	if(device->get().createPipelineLayout(&layout_information,
		nullptr, &pipeline_layout) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline layout.");

	//Pipeline
	vk::GraphicsPipelineCreateInfo pipeline_information;
	pipeline_information.setStageCount(static_cast<uint32_t>(shader.get().size()));
	pipeline_information.setPStages(shader.get().data());
	pipeline_information.setPVertexInputState(&vertex_input_state_information);
	pipeline_information.setPInputAssemblyState(&input_assembly_state_information);
	pipeline_information.setPTessellationState(nullptr);
	pipeline_information.setPViewportState(&viewport_state_information);
	pipeline_information.setPRasterizationState(&rasterization_state_information);
	pipeline_information.setPMultisampleState(&multisample_state_information);
	pipeline_information.setPDepthStencilState(&depth_stencil_information);
	pipeline_information.setPColorBlendState(&color_blend_state_information);
	pipeline_information.setPDynamicState(nullptr);
	pipeline_information.setLayout(pipeline_layout);
	pipeline_information.setRenderPass(render_pass.get());
	pipeline_information.setSubpass(0);
	pipeline_information.setBasePipelineHandle(base.get());
	pipeline_information.setBasePipelineIndex(-1);

	if(device->get().createGraphicsPipelines(nullptr, 1,
		&pipeline_information, nullptr, pipeline.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
}

Oreginum::Vulkan::Pipeline::~Pipeline()
{
	if(pipeline.use_count() != 1 || !device) return;
	if(*pipeline) device->get().destroyPipeline(*pipeline);
	if(pipeline_layout) device->get().destroyPipelineLayout(pipeline_layout);
}

void Oreginum::Vulkan::Pipeline::swap(Pipeline *other)
{
	std::swap(device, other->device);
	std::swap(pipeline_layout, other->pipeline_layout);
	std::swap(pipeline, other->pipeline);
	std::swap(descriptor_set_layouts, other->descriptor_set_layouts);
}