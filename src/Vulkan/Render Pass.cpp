#include "../Oreginum/Core.hpp"
#include "Swapchain.hpp"
#include "Render Pass.hpp"

Oreginum::Vulkan::Render_Pass::Render_Pass(std::shared_ptr<Device> device,
	const std::vector<vk::AttachmentDescription>& attachments,
	const std::vector<vk::SubpassDescription>& subpasses,
	const std::vector<vk::SubpassDependency>& dependencies) : device(device)
{
	//Render pass
	vk::RenderPassCreateInfo render_pass_information{{},
		static_cast<uint32_t>(attachments.size()), attachments.data(),
		static_cast<uint32_t>(subpasses.size()), subpasses.data(),
		static_cast<uint32_t>(dependencies.size()), dependencies.data()};

	if(device->get().createRenderPass(&render_pass_information,
		nullptr, render_pass.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan render pass.");
}

Oreginum::Vulkan::Render_Pass::~Render_Pass()
{
	if(render_pass.use_count() != 1 || !*render_pass) return;
	device->get().destroyRenderPass(*render_pass);
}

void Oreginum::Vulkan::Render_Pass::swap(Render_Pass *other)
{
	std::swap(device, other->device);
	std::swap(render_pass, other->render_pass);
}