#include "../Oreginum/Core.hpp"
#include "Buffer.hpp"

Oreginum::Vulkan::Buffer::Buffer(std::shared_ptr<const Device> device,
	const Command_Buffer& temporary_command_buffer, vk::BufferUsageFlags usage,
	size_t size, const void *data, size_t uniform_size) : device(device), 
	temporary_command_buffer(&temporary_command_buffer), size(size),
	descriptor_type(uniform_size ? vk::DescriptorType::eUniformBufferDynamic :
		vk::DescriptorType::eUniformBuffer)
{
	create_buffer(&stage, &stage_memory, size, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	create_buffer(buffer.get(), &buffer_memory, size, usage |
		vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
	if(data) write(data, size);
	descriptor_information = {*buffer, 0, uniform_size ? uniform_size : size};
}

Oreginum::Vulkan::Buffer::~Buffer()
{
	if(buffer.use_count() != 1 || !device) return;
	if(*buffer) vkDestroyBuffer(device->get(), *buffer, nullptr);
	if(buffer_memory) vkFreeMemory(device->get(), buffer_memory, nullptr);
	if(stage) vkDestroyBuffer(device->get(), stage, nullptr);
	if(stage_memory) vkFreeMemory(device->get(), stage_memory, nullptr);
}

void Oreginum::Vulkan::Buffer::swap(Buffer *other)
{
	std::swap(device, other->device);
	std::swap(temporary_command_buffer, other->temporary_command_buffer);
	std::swap(size, other->size);
	std::swap(buffer, other->buffer);
	std::swap(buffer_memory, other->buffer_memory);
	std::swap(stage, other->stage);
	std::swap(stage_memory, other->stage_memory);
	std::swap(descriptor_information, other->descriptor_information);
	std::swap(descriptor_type, other->descriptor_type);
}

void Oreginum::Vulkan::Buffer::create_buffer(vk::Buffer *buffer, vk::DeviceMemory *memory,
	size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_property_flags)
{
	//Create buffer
	vk::BufferCreateInfo buffer_information{{}, size, usage};
	if(device->get().createBuffer(&buffer_information, nullptr, buffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan buffer.");

	//Allocate buffer memory
	vk::MemoryRequirements memory_requirements
	(device->get().getBufferMemoryRequirements(*buffer));
	vk::MemoryAllocateInfo memory_information{memory_requirements.size,
		find_memory(*device, memory_requirements.memoryTypeBits, memory_property_flags)};
	if(device->get().allocateMemory(&memory_information, nullptr, memory) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate memory for a Vulkan buffer.");

	//Bind buffer memory
	if(device->get().bindBufferMemory(*buffer, *memory, 0) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not bind memory to a Vulkan buffer.");
}

uint32_t Oreginum::Vulkan::Buffer::find_memory(const Device& device,
	uint32_t type, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memory_properties(device.get_gpu().getMemoryProperties());
	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags &
			properties) == properties) return i;
	Oreginum::Core::error("Could not find suitable Vulkan memory.");
	return NULL;
}

void Oreginum::Vulkan::Buffer::write(const void *data, size_t size, size_t offset)
{
	//Copy data to stage buffer
	this->size = size;
	auto result{device->get().mapMemory(stage_memory, offset, size)};
	if(result.result != vk::Result::eSuccess)
		Oreginum::Core::error("Could not map Vulkan buffer stage memory.");
	std::memcpy(result.value, data, size);
	device->get().unmapMemory(stage_memory);

	//Copy data from stage buffer to device buffer
	temporary_command_buffer->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	temporary_command_buffer->get().copyBuffer(stage, *buffer, {{offset, offset, size}});
	temporary_command_buffer->end_and_submit();
}