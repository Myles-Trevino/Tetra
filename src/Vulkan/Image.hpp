#pragma once
#include <GLM/glm.hpp>
#include "Device.hpp"
#include "Command Buffer.hpp"
#include "Sampler.hpp"
#include "Uniform.hpp"

namespace Oreginum::Vulkan
{
	class Image : public Uniform
	{
	public:
		static constexpr vk::Format SWAPCHAIN_FORMAT{vk::Format::eB8G8R8A8Unorm};
		static constexpr vk::ColorSpaceKHR SWAPCHAIN_COLOR_SPACE{vk::ColorSpaceKHR::eSrgbNonlinear};
		static constexpr vk::Format LINEAR_FORMAT{vk::Format::eR8G8B8A8Srgb};
		static constexpr vk::Format RGB_FORMAT{vk::Format::eR8G8B8A8Unorm};
		static constexpr vk::Format MONOCHROME_FORMAT{vk::Format::eR8Unorm};
		static constexpr vk::Format HDR_FORMAT_32{vk::Format::eR32G32B32A32Sfloat};
		static constexpr vk::Format HDR_FORMAT{vk::Format::eR16G16B16A16Sfloat};
		static constexpr vk::Format DEPTH_FORMAT{vk::Format::eD32Sfloat};
		static constexpr vk::Format POSITION_FORMAT{vk::Format::eR16G16B16A16Sfloat};
		static const vk::FormatFeatureFlags DEPTH_FEATURES;

		Image(){}
		Image(std::shared_ptr<Device> device, const Sampler& sampler, const glm::uvec2& resolution,
			vk::ImageUsageFlags usage, vk::Format format, vk::ImageAspectFlags aspect,
			vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
		Image(std::shared_ptr<Device> device, const Sampler& sampler,
			const Command_Buffer& temporary_command_buffer,
			const glm::uvec2& resolution, const std::vector<void *>& datas,
			vk::Format format, bool cubemap);
		Image(std::shared_ptr<Device> device, vk::Image image, vk::Format format = SWAPCHAIN_FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor)
			: device(device), image(image), aspect(aspect), swapchain(true)
		{  create_image_view(format, aspect); }
		Image *operator=(Image other){ swap(&other); return this; }
		~Image();

		void transition(const Command_Buffer& command_buffer, const vk::Image& image,
			vk::ImageAspectFlags aspect, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
			vk::AccessFlags source_access_flags, vk::AccessFlags destination_access_flags,
			vk::PipelineStageFlags source_stage, vk::PipelineStageFlags destination_stage, 
			uint32_t base_layer = 0, uint32_t layers = 1, uint32_t base_level = 0, uint32_t levels = 1,
			uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED,
			bool temporary = true) const;
		void transition(const Command_Buffer& command_buffer, vk::ImageLayout old_layout,
			vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
			vk::AccessFlags destination_access_flags, vk::PipelineStageFlags source_stage,
			vk::PipelineStageFlags destination_stage, bool temporary = true) const
		{
			transition(command_buffer, image, aspect, old_layout, new_layout,
				source_access_flags, destination_access_flags, source_stage, destination_stage,
				0, 1, 0, 1, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, temporary);
		}

		const vk::Image& get() const { return image; }
		const vk::ImageView& get_view() const { return *image_view; }
        const glm::uvec2& get_resolution() const { return resolution; }
        uint8_t get_mip_levels() const { return mip_levels; }

		Descriptor_Information get_descriptor_information() const
		{ return {vk::DescriptorType::eCombinedImageSampler, &descriptor_information, nullptr}; }

	private:
		std::shared_ptr<Device> device;
		vk::Image image;
		vk::DeviceMemory image_memory;
		std::shared_ptr<vk::ImageView> image_view = std::make_shared<vk::ImageView>();
		vk::ImageAspectFlags aspect;
        glm::uvec2 resolution;
		uint8_t mip_levels;
		vk::DescriptorImageInfo descriptor_information;
		bool swapchain{false};

		void swap(Image *other);
		vk::Image create_image(const Device& device, const glm::uvec2& resolution,
			uint32_t mip_levels, vk::ImageUsageFlags usage, vk::Format format,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, uint32_t layers = 1,
			vk::ImageTiling tiling = vk::ImageTiling::eOptimal, bool array_2d = false,
			bool cubemap = false, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
		vk::DeviceMemory create_and_bind_image_memory(const Device& device,
			const vk::Image& image, vk::MemoryPropertyFlags flags =
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		void copy_image(const Command_Buffer& temporary_command_buffer, const vk::Image& source,
			const vk::Image& destination, const glm::uvec2& resolution, uint32_t layer = 0,
			uint32_t level = 0, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
		void create_image_view(vk::Format format,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
			uint32_t layers = 1, uint32_t levels = 1,
			vk::ImageViewType view_type = vk::ImageViewType::e2D);
	};
}