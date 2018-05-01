#pragma once
#include "Device.hpp"
#include "Descriptor Pool.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"
#include "Uniform.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Set
	{
	public:
		Descriptor_Set(){}
		Descriptor_Set(std::shared_ptr<Device> device, const Descriptor_Pool& pool,
			const std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>& bindings);
		Descriptor_Set *operator=(Descriptor_Set other){ swap(&other); return this; }
		~Descriptor_Set();

		void write(const std::vector<const Uniform *>& uniforms);

		const vk::DescriptorSet& get() const { return descriptor_set; }
		const vk::DescriptorSetLayout& get_layout() const { return *layout; }

	private:
		std::shared_ptr<Device> device;
		vk::DescriptorType type;
		vk::DescriptorSet descriptor_set;
		std::shared_ptr<vk::DescriptorSetLayout> layout =
			std::make_shared<vk::DescriptorSetLayout>();

		void swap(Descriptor_Set *other);
	};
}