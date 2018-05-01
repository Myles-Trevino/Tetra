#include <iostream>
#include <map>
#include <set>
#include "../Oreginum/Core.hpp"
#include "Swapchain.hpp"
#include "Device.hpp"

Oreginum::Vulkan::Device::Device(const Instance& instance, const Surface& surface)
	: instance(std::make_shared<const Instance>(instance)), surface(&surface)
{
	select_gpu();
	create_device();
}

Oreginum::Vulkan::Device::~Device()
{
	if(device.use_count() != 1 || !*device) return;
	device->destroy();
}

void Oreginum::Vulkan::Device::swap(Device *other)
{
	std::swap(instance, other->instance);
	std::swap(surface, other->surface);
	std::swap(gpu_extensions, other->gpu_extensions);
	std::swap(supported_gpu_extensions, other->supported_gpu_extensions);
	std::swap(graphics_queue_family_index, other->graphics_queue_family_index);
	std::swap(present_queue_family_index, other->present_queue_family_index);
	std::swap(graphics_queue, other->graphics_queue);
	std::swap(present_queue, other->present_queue);
	std::swap(gpu_properties, other->gpu_properties);
	std::swap(gpu_features, other->gpu_features);
	std::swap(surface_capabilities, other->surface_capabilities);
	std::swap(surface_formats, other->surface_formats);
	std::swap(swapchain_present_modes, other->swapchain_present_modes);
	std::swap(gpu, other->gpu);
	std::swap(device, other->device);
}

void Oreginum::Vulkan::Device::get_gpu_swapchain_information(const vk::PhysicalDevice& gpu)
{
	gpu.getSurfaceCapabilitiesKHR(surface->get(), &surface_capabilities);
	surface_formats = gpu.getSurfaceFormatsKHR(surface->get()).value;
	swapchain_present_modes = gpu.getSurfacePresentModesKHR(surface->get()).value;
}

void Oreginum::Vulkan::Device::get_gpu_information(const vk::PhysicalDevice& gpu)
{
	//General
	gpu.getProperties(&gpu_properties);
	gpu.getFeatures(&gpu_features);

	//Extensions
	supported_gpu_extensions = gpu.enumerateDeviceExtensionProperties().value;

	//Graphics and present queues
	std::vector<vk::QueueFamilyProperties> queue_family_properties
	{gpu.getQueueFamilyProperties()};

	graphics_queue_family_index = UINT32_MAX, present_queue_family_index = UINT32_MAX;
	for(uint32_t i{}; i < queue_family_properties.size(); ++i)
	{
		if(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			graphics_queue_family_index = i;
		vk::Bool32 surface_supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(static_cast<VkPhysicalDevice>(gpu),
			i, static_cast<VkSurfaceKHR>(surface->get()), &surface_supported);
		if(surface_supported) present_queue_family_index = i;
		if(graphics_queue_family_index == present_queue_family_index) break;
	}

	//Swapchain
	get_gpu_swapchain_information(gpu);
}

void Oreginum::Vulkan::Device::select_gpu()
{
	std::vector<vk::PhysicalDevice> gpus{instance->get().enumeratePhysicalDevices().value};

	std::map<int, vk::PhysicalDevice> gpu_ratings;
	for(const auto& g : gpus)
	{
		uint32_t rating{};
		get_gpu_information(g);

		//GPU type
		if(gpu_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) rating += 2;

		//Extensions
		std::set<std::string> required_extensions(gpu_extensions.begin(), gpu_extensions.end());
		for(const auto& e : supported_gpu_extensions)
			required_extensions.erase(e.extensionName);
		if(!required_extensions.empty()) continue;

		//Graphics queue
		if(graphics_queue_family_index == UINT32_MAX ||
			present_queue_family_index == UINT32_MAX) continue;
		if(graphics_queue_family_index == present_queue_family_index) rating += 1;

		//Swapchain minimum image count
		if((surface_capabilities.maxImageCount > 0) && (Swapchain::MINIMUM_IMAGE_COUNT >
			surface_capabilities.maxImageCount)) continue;

		//Swapchain format
		bool swapchain_format_supported{};
		if(!((surface_formats.size() == 1) &&
			(surface_formats[0].format == vk::Format::eUndefined)))
		{
			for(const auto& f : surface_formats)
				if(f.format == Image::SWAPCHAIN_FORMAT && f.colorSpace ==
					Image::SWAPCHAIN_COLOR_SPACE) swapchain_format_supported = true;
			if(!swapchain_format_supported) continue;
		}

		//Swapchain present mode
		bool swapchain_present_mode_supported{};
		for(const auto& p : swapchain_present_modes)
			if(p == vk::PresentModeKHR::eMailbox) swapchain_present_mode_supported = true;
		if(!swapchain_present_mode_supported) continue;

		//Depth format
		vk::FormatProperties properties(g.getFormatProperties(Image::DEPTH_FORMAT));
		if((properties.optimalTilingFeatures & Image::DEPTH_FEATURES)
			!= Image::DEPTH_FEATURES) continue;

		gpu_ratings.insert({rating, g});
	}

	if(gpu_ratings.empty())
		Oreginum::Core::error("Could not find a GPU that supports Vulkan sufficiently.");
	gpu = gpu_ratings.begin()->second;
	get_gpu_information(gpu);
}

void Oreginum::Vulkan::Device::create_device()
{
	std::vector<vk::DeviceQueueCreateInfo> device_queue_informations;
	static constexpr float QUEUE_PRIORITY{1};
	std::set<uint32_t> unique_queues{graphics_queue_family_index, present_queue_family_index};
	for(uint32_t q : unique_queues) device_queue_informations.push_back(
		vk::DeviceQueueCreateInfo{{}, q, 1, &QUEUE_PRIORITY});

	vk::PhysicalDeviceFeatures features{};
	features.setSamplerAnisotropy(true);
	features.setShaderStorageImageMultisample(true);
	features.setSampleRateShading(true);
	vk::DeviceCreateInfo device_information{{},
		static_cast<uint32_t>(device_queue_informations.size()),
		device_queue_informations.data(), 0, nullptr,
		static_cast<uint32_t>(gpu_extensions.size()), gpu_extensions.data(), &features};

	if(gpu.createDevice(&device_information, nullptr, device.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan device.");

	graphics_queue = device->getQueue(graphics_queue_family_index, 0);
	present_queue = device->getQueue(present_queue_family_index, 0);
}