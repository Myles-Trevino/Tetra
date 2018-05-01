#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Command Buffer.hpp"
#include "Uniform.hpp"

namespace Oreginum::Vulkan
{
	class Buffer : public Uniform
	{
	public:
		Buffer(){}
		Buffer(std::shared_ptr<const Device> device, const Command_Buffer& temporary_command_buffer,
			vk::BufferUsageFlags usage, size_t size, const void *data = nullptr, 
			size_t uniform_size = 0);
		Buffer *operator=(Buffer other){ swap(&other); return this; }
		~Buffer();

		static uint32_t find_memory(const Device& device, uint32_t type,
			vk::MemoryPropertyFlags properties);
		void write(const void *data = nullptr, size_t size = 0, size_t offset = 0);

		const vk::Buffer& get() const { return *buffer; }
		size_t get_size() const { return size; }

		Descriptor_Information get_descriptor_information() const
		{ return {descriptor_type, nullptr, &descriptor_information}; }

	private:
		std::shared_ptr<const Device> device;
		const Command_Buffer *temporary_command_buffer;
		size_t size;
		std::shared_ptr<vk::Buffer> buffer = std::make_shared<vk::Buffer>();
		vk::DeviceMemory buffer_memory;
		vk::Buffer stage;
		vk::DeviceMemory stage_memory;
		vk::DescriptorBufferInfo descriptor_information;
		vk::DescriptorType descriptor_type;

		void swap(Buffer *other);
		void create_buffer(vk::Buffer *buffer, vk::DeviceMemory *memory, size_t size,
			vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_property_flags);
	};
}