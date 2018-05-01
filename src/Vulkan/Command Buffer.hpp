#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Fence.hpp"
#include "Device.hpp"
#include "Command Pool.hpp"

namespace Oreginum::Vulkan
{
	class Command_Buffer
	{
	public:
		Command_Buffer(){}
		Command_Buffer(std::shared_ptr<Device> device, const Command_Pool& command_pool,
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		Command_Buffer *operator=(Command_Buffer other){ swap(&other); return this; }
		~Command_Buffer();

		void begin(vk::CommandBufferUsageFlagBits flags =
			vk::CommandBufferUsageFlagBits::eSimultaneousUse) const;
		void end() const;
		void submit() const;
		void end_and_submit() const { end(), submit(); }

		const vk::CommandBuffer& get() const { return *command_buffer; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<Command_Pool> command_pool = std::make_shared<Command_Pool>();
		std::shared_ptr<vk::CommandBuffer> command_buffer =
			std::make_shared<vk::CommandBuffer>();
        Fence fence;

		void swap(Command_Buffer *other);
	};
}