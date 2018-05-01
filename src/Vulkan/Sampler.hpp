#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Sampler
	{
	public:
		Sampler(){};
		Sampler(std::shared_ptr<Device> device, uint8_t lod = 0, bool anisotropy = false,
			vk::SamplerAddressMode address_mode = vk::SamplerAddressMode::eRepeat,
			vk::Filter close_filter = vk::Filter::eLinear,
			vk::Filter far_filter = vk::Filter::eLinear,
			vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eLinear);
		Sampler *operator=(Sampler other){ swap(&other); return this; }
		~Sampler();

		const vk::Sampler& get() const { return *sampler; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::Sampler> sampler = std::make_shared<vk::Sampler>();

		void swap(Sampler *other);
	};
}