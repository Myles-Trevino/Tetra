#include "../Oreginum/Core.hpp"
#include "Command Pool.hpp"

Oreginum::Vulkan::Command_Pool::Command_Pool(std::shared_ptr<Device> device,
	 uint32_t queue_family_index, vk::CommandPoolCreateFlags flags) : device(device)
{
	vk::CommandPoolCreateInfo pool_information{flags, queue_family_index};
	if(device->get().createCommandPool(&pool_information, nullptr, command_pool.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan command pool.");
}

Oreginum::Vulkan::Command_Pool::~Command_Pool()
{
	if(command_pool.use_count() != 1 || !*command_pool) return;
	device->get().destroyCommandPool(*command_pool);
}

void Oreginum::Vulkan::Command_Pool::swap(Command_Pool *other)
{
	std::swap(device, other->device);
	std::swap(command_pool, other->command_pool);
}
