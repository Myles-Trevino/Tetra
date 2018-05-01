#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Semaphore
	{
	public:
		Semaphore(){}
		Semaphore(std::shared_ptr<Device> device);
		Semaphore *operator=(Semaphore other){ swap(&other); return this; }
		~Semaphore();

		const vk::Semaphore& get() const { return *semaphore; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<vk::Semaphore> semaphore = std::make_shared<vk::Semaphore>();

		void swap(Semaphore *other);
	};
}