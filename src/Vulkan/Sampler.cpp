#include "../Oreginum/Core.hpp"
#include "Sampler.hpp"

Oreginum::Vulkan::Sampler::Sampler(std::shared_ptr<Device> device, uint8_t lod, bool anisotropy,
	vk::SamplerAddressMode address_mode, vk::Filter close_filter, vk::Filter far_filter,
	vk::SamplerMipmapMode mipmap_mode) : device(device)
{
	vk::SamplerCreateInfo sampler_information{{}, close_filter, far_filter, mipmap_mode,
		address_mode, address_mode, address_mode, 0, anisotropy, anisotropy ? 16.f : 1.f, VK_FALSE,
		vk::CompareOp::eNever, 0, static_cast<float>(lod), vk::BorderColor::eFloatTransparentBlack,
		VK_FALSE};
	if(device->get().createSampler(&sampler_information, nullptr, sampler.get()) !=
		vk::Result::eSuccess || !*sampler) Core::error("Could not create a Vulkan sampler.");
}

Oreginum::Vulkan::Sampler::~Sampler()
{
	if(sampler.use_count() != 1 || !*sampler || !device) return;
	device->get().destroySampler(*sampler);
};

void Oreginum::Vulkan::Sampler::swap(Sampler *other)
{
	std::swap(device, other->device);
	std::swap(sampler, other->sampler);
}