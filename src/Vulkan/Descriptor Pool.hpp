#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Descriptor_Pool
	{
	public:
		Descriptor_Pool(){}
		Descriptor_Pool(std::shared_ptr<Device> device,
			const std::vector<std::pair<vk::DescriptorType, uint32_t>>& sizes);
		Descriptor_Pool *operator=(Descriptor_Pool other){ swap(&other); return this; }
		~Descriptor_Pool();

		const vk::DescriptorPool& get() const { return *descriptor_pool; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::DescriptorPool> descriptor_pool = 
			std::make_shared<vk::DescriptorPool>();

		void swap(Descriptor_Pool *other);
	};
}