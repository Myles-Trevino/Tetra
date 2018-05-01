#pragma once
#include "Instance.hpp"

namespace Oreginum::Vulkan
{
	class Surface
	{
	public:
		Surface(){}
		Surface(std::shared_ptr<Instance> instance);
		Surface *operator=(Surface other){ swap(&other); return this; }
		~Surface();

		vk::SurfaceKHR get() const { return *surface; }

	private:
		std::shared_ptr<Instance> instance;
		std::shared_ptr<vk::SurfaceKHR> surface = std::make_shared<vk::SurfaceKHR>();

		void swap(Surface *other);
	};
}