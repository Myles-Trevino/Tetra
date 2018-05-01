#pragma once
#include "Device.hpp"
#include "Render Pass.hpp"
#include "Image.hpp"
#include "Swapchain.hpp"

namespace Oreginum::Vulkan
{
	class Framebuffer
	{
	public:
		Framebuffer(){}
		Framebuffer(std::shared_ptr<Device> device, const glm::uvec2& resolution,
			const Render_Pass& render_pass, const std::vector<const Image *>& attachments);
		Framebuffer *operator=(Framebuffer other){ swap(&other); return this; }
		~Framebuffer();

		const vk::Framebuffer& get() const { return *framebuffer; }
		const glm::uvec2& get_resolution() const { return resolution; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::Framebuffer> framebuffer = std::make_shared<vk::Framebuffer>();
        glm::uvec2 resolution;

		void swap(Framebuffer *other);
	};
}