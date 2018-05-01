#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Fence
	{
	public:
        Fence(){}
		Fence(std::shared_ptr<Device> device, vk::FenceCreateFlags flags =
			vk::FenceCreateFlagBits::eSignaled);
		Fence *operator=(Fence other){ swap(&other); return this; }
		~Fence();

		const vk::Fence& get() const { return *fence; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::Fence> fence = std::make_shared<vk::Fence>();

		void swap(Fence *other);
	};
}