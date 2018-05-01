#include "../Oreginum/Core.hpp"
#include "Descriptor Pool.hpp"

Oreginum::Vulkan::Descriptor_Pool::Descriptor_Pool(std::shared_ptr<Device> device,
	const std::vector<std::pair<vk::DescriptorType, uint32_t>>& sizes) : device(device)
{
	uint32_t descriptor_set_count{};
	std::vector<vk::DescriptorPoolSize> pool_sizes{sizes.size()};
	for(uint32_t i{}; i < sizes.size(); ++i)
	{
		descriptor_set_count += sizes[i].second;
		pool_sizes[i] = {sizes[i].first, sizes[i].second};
	}

	vk::DescriptorPoolCreateInfo descriptor_pool_information{{}, descriptor_set_count,
		static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data(), };

	if(device->get().createDescriptorPool(&descriptor_pool_information,
		nullptr, descriptor_pool.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan descriptor pool.");
}

Oreginum::Vulkan::Descriptor_Pool::~Descriptor_Pool()
{
	if(descriptor_pool.use_count() != 1 || !*descriptor_pool) return;
	device->get().destroyDescriptorPool(*descriptor_pool);
}

void Oreginum::Vulkan::Descriptor_Pool::swap(Descriptor_Pool *other)
{
	std::swap(device, other->device);
	std::swap(descriptor_pool, other->descriptor_pool);
}
