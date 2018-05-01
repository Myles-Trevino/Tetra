#pragma once
#include <vector>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>

namespace Oreginum::Vulkan
{
	class Instance
	{
	public:
		Instance(){};
		Instance(bool debug);
		Instance *operator=(Instance other){ swap(&other); return this; }
		~Instance();

		const vk::Instance& get() const { return *instance; }

	private:
		std::vector<const char *> instance_extensions
		{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		std::vector<const char *> instance_layers{};
		std::shared_ptr<vk::Instance> instance = std::make_shared<vk::Instance>();
		vk::DebugReportCallbackEXT debug_callback;

		void swap(Instance *other);
		void create_debug_callback();
	};
}