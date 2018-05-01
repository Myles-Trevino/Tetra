#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Command_Pool
	{
	public:
		Command_Pool(){}
		Command_Pool(std::shared_ptr<Device> device, uint32_t queue_family_index,
			vk::CommandPoolCreateFlags flags = {});
		Command_Pool *operator=(Command_Pool other){ swap(&other); return this; }
		~Command_Pool();

		const vk::CommandPool& get() const { return *command_pool; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::CommandPool> command_pool = std::make_shared<vk::CommandPool>();

		void swap(Command_Pool *other);
	};
}