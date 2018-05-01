#include <iostream>
#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Instance.hpp"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
	auto fvkCreateDebugReportCallbackEXT{reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"))};
	return fvkCreateDebugReportCallbackEXT( instance, pCreateInfo, pAllocator, pCallback );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance,
	VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator)
{
	auto fvkDestroyDebugReportCallbackEXT{reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"))};
	fvkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_function(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT type, uint64_t object, size_t location,
	int32_t code, const char *layer_prefix, const char *message, void *user_data)
{
 	std::cout<<layer_prefix<<": "<<message<<'\n';
	return false;
}

Oreginum::Vulkan::Instance::Instance(bool debug)
{
	vk::ApplicationInfo application_information
	{Oreginum::Window::get_title().c_str(), VK_MAKE_VERSION(0, 0, 1), "Oreginum Engine",
		VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_0};

	if(debug) instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME),
		instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");

	vk::InstanceCreateInfo instance_information{{}, &application_information,
		static_cast<uint32_t>(instance_layers.size()), instance_layers.data(),
		static_cast<uint32_t>(instance_extensions.size()), instance_extensions.data()};

	if(vk::createInstance(&instance_information, nullptr, instance.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Vulkan is not supported sufficiently.");

	if(debug) create_debug_callback();
}

Oreginum::Vulkan::Instance::~Instance()
{
	if(instance.use_count() != 1) return;
	if(debug_callback) instance->destroyDebugReportCallbackEXT(debug_callback, nullptr);
	if(*instance) instance->destroy();
}

void Oreginum::Vulkan::Instance::swap(Instance *other)
{
	std::swap(other->instance_extensions, this->instance_extensions);
	std::swap(other->instance_layers, this->instance_layers);
	std::swap(other->instance, this->instance);
	std::swap(other->debug_callback, this->debug_callback);
}

void Oreginum::Vulkan::Instance::create_debug_callback()
{
	vk::DebugReportCallbackCreateInfoEXT debug_callback_information
	{vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning |
		vk::DebugReportFlagBitsEXT::ePerformanceWarning, debug_callback_function};

	if(instance->createDebugReportCallbackEXT(&debug_callback_information,
		nullptr, &debug_callback) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not initialize Vulkan debugging.");
}