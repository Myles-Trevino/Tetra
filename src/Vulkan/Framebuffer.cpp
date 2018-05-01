#include "../Oreginum/Core.hpp"
#include "Framebuffer.hpp"

Oreginum::Vulkan::Framebuffer::Framebuffer(std::shared_ptr<Device> device,
	const glm::uvec2& resolution, const Render_Pass& render_pass,
	const std::vector<const Image *>& attachments)
    : device(device), resolution(resolution)
{
	std::vector<vk::ImageView> attachment_views;
	for(const Image *a : attachments) attachment_views.emplace_back(a->get_view());

	vk::FramebufferCreateInfo framebuffer_information{{}, render_pass.get(),
		static_cast<uint32_t>(attachment_views.size()), attachment_views.data(),
		resolution.x, resolution.y, 1};

	if(device->get().createFramebuffer(&framebuffer_information,
		nullptr, framebuffer.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan framebuffer.");
}

Oreginum::Vulkan::Framebuffer::~Framebuffer()
{
	if(framebuffer.use_count() != 1 || !*framebuffer) return;
	device->get().destroyFramebuffer(*framebuffer);
}

void Oreginum::Vulkan::Framebuffer::swap(Framebuffer *other)
{
	std::swap(device, other->device);
	std::swap(framebuffer, other->framebuffer);
	std::swap(resolution, other->resolution);
}