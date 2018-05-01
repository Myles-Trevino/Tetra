#pragma once
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Image.hpp"

namespace Oreginum::Vulkan
{
	class Swapchain
	{
	public:
		static constexpr uint32_t MINIMUM_IMAGE_COUNT{3};
		static constexpr vk::SampleCountFlagBits SAMPLES{vk::SampleCountFlagBits::e4};
		static constexpr bool MULTISAMPLE{SAMPLES != vk::SampleCountFlagBits::e1};

		Swapchain(){};
		Swapchain(const Instance& instance, std::shared_ptr<Surface> surface,
			std::shared_ptr<Device> device, const Command_Buffer& command_buffer)
			: instance(&instance), surface(surface), device(device)
		{ initialize(instance, surface, device, command_buffer); }
		Swapchain *operator=(Swapchain other){ swap(&other); return this; }
		~Swapchain();

		void reinitialize(std::shared_ptr<Device> device, const Command_Buffer& command_buffer)
		{ initialize(*instance, surface, device, command_buffer); }

		const vk::SwapchainKHR& get() const { return *swapchain; }
		const std::vector<Image>& get_images() const { return images; }
		const glm::uvec2& get_resolution() const { return {extent.width, extent.height}; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<Surface> surface;
		const Instance *instance;
		vk::Extent2D extent;
		std::shared_ptr<vk::SwapchainKHR> swapchain = std::make_shared<vk::SwapchainKHR>();
		std::vector<Image> images;

		void swap(Swapchain *other);
		void initialize(const Instance& instance, std::shared_ptr<Surface> surface,
			std::shared_ptr<Device> device, const Command_Buffer& command_buffer);
	};
}