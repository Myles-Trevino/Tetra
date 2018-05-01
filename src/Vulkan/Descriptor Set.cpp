#include <iostream>
#include "../Oreginum/Core.hpp"
#include "Descriptor Set.hpp"

Oreginum::Vulkan::Descriptor_Set::Descriptor_Set(std::shared_ptr<Device> device,
	const Descriptor_Pool& pool,
	const std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>>& bindings)
	: device(device), type(type)
{
	//Create layout
	std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings;
	for(uint32_t i{}; i < bindings.size(); ++i)
		descriptor_set_layout_bindings.push_back({i, bindings[i].first, 1, bindings[i].second});

	vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_information
	{{}, static_cast<uint32_t>(descriptor_set_layout_bindings.size()),
		descriptor_set_layout_bindings.data()};
	if(device->get().createDescriptorSetLayout(&descriptor_set_layout_information,
		nullptr, layout.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan descriptor set layout.");

	//Allocate set
	vk::DescriptorSetAllocateInfo descriptor_set_allocate_information{pool.get(), 1, layout.get()};
	if(device->get().allocateDescriptorSets(&descriptor_set_allocate_information, &descriptor_set) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not allocate a Vulkan descriptor set.");
}

Oreginum::Vulkan::Descriptor_Set::~Descriptor_Set()
{
	if(layout.use_count() != 1 || !*layout) return;
	device->get().destroyDescriptorSetLayout(*layout);
}

void Oreginum::Vulkan::Descriptor_Set::write(const std::vector<const Uniform *>& uniforms)
{
	std::vector<vk::WriteDescriptorSet> write_descriptor_sets;

	for(uint32_t i{}; i < uniforms.size(); ++i)
	{
		write_descriptor_sets.push_back({descriptor_set, i, 0, 1,
			uniforms[i]->get_descriptor_information().type,
			uniforms[i]->get_descriptor_information().image,
			uniforms[i]->get_descriptor_information().buffer});
	}
	
	device->get().updateDescriptorSets(write_descriptor_sets, {});
}

void Oreginum::Vulkan::Descriptor_Set::swap(Descriptor_Set *other)
{
	std::swap(device, other->device);
	std::swap(type, other->type);
	std::swap(descriptor_set, other->descriptor_set);
	std::swap(layout, other->layout);
}