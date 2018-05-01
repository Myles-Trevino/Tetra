#include <mutex>
#include "Core.hpp"
#include "Window.hpp"
#include "Renderable.hpp"
#include "Main Renderer.hpp"
#include "Renderer Core.hpp"

namespace
{
	std::shared_ptr<Oreginum::Vulkan::Instance> instance;
	std::shared_ptr<Oreginum::Vulkan::Surface> surface;
	std::shared_ptr<Oreginum::Vulkan::Device> device;
	Oreginum::Vulkan::Command_Pool temporary_command_pool;
	Oreginum::Vulkan::Command_Buffer temporary_command_buffer;
	Oreginum::Vulkan::Descriptor_Pool static_descriptor_pool, descriptor_pool;
	Oreginum::Vulkan::Command_Pool command_pool;
	uint32_t uniform_size, padded_uniform_size, uniform_buffer_size;
	std::map<Oreginum::Renderer_Core::Key, Oreginum::Renderable *> renderables;
	Oreginum::Vulkan::Descriptor_Set uniform_descriptor_set, texture_descriptor_set;
	Oreginum::Vulkan::Buffer uniform_buffer;
	bool rerecord{true};
	uint32_t id;
    std::mutex render_mutex;
	uint32_t minimum_offset;
}

void Oreginum::Renderer_Core::submit_command_buffers(
	const std::vector<vk::CommandBuffer>& command_buffers,
	const std::vector<vk::Semaphore>& wait_semaphores,
	const std::vector<vk::PipelineStageFlags>& wait_stages,
	const std::vector<vk::Semaphore>& signal_semaphores)
{
	vk::SubmitInfo submit_information{static_cast<uint32_t>(wait_semaphores.size()),
		wait_semaphores.data(), wait_stages.data(), static_cast<uint32_t>(
			command_buffers.size()), command_buffers.data(),
		static_cast<uint32_t>(signal_semaphores.size()), signal_semaphores.data()};
	if(Oreginum::Renderer_Core::get_device()->get_graphics_queue().submit(
        submit_information, nullptr) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not submit buffer renderer Vulkan command buffer.");
}

Oreginum::Vulkan::Pipeline Oreginum::Renderer_Core::create_pipeline(
	const glm::ivec2& resolution, const Vulkan::Render_Pass& render_pass,
	const std::string& vertex, const std::string& fragment, uint8_t render_pass_number,
	const std::vector<vk::DescriptorSetLayout>& descriptor_layouts,
	const Vulkan::Pipeline& base)
{
	Vulkan::Shader shader{device, {{vertex, vk::ShaderStageFlagBits::eVertex},
		{fragment, vk::ShaderStageFlagBits::eFragment}}};
	return {device, resolution, render_pass, shader,
		descriptor_layouts, render_pass_number, base};
}

uint32_t Oreginum::Renderer_Core::get_padded_uniform_size(uint32_t uniform_size)
{
	uint32_t offset_difference{uniform_size%minimum_offset};
	uint32_t padded_uniform_size{uniform_size};
	if(offset_difference) padded_uniform_size += minimum_offset-offset_difference;
	return padded_uniform_size;
}

void Oreginum::Renderer_Core::initialize()
{
	if(instance.get()) return;

	//Initialization
	instance = std::make_shared<Vulkan::Instance>(Core::get_debug());
	surface = std::make_shared<Vulkan::Surface>(instance);
	device = std::make_shared<Vulkan::Device>(*instance, *surface);
	temporary_command_pool = {device, device->get_graphics_queue_family_index(),
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer};
	temporary_command_buffer = {device, temporary_command_pool};
	command_pool = {device, device->get_graphics_queue_family_index()};

	//Calculate uniform buffer padding
	minimum_offset = static_cast<uint32_t>(device->
		 get_properties().limits.minUniformBufferOffsetAlignment);
	uniform_size = static_cast<uint32_t>(sizeof(Renderable::Uniforms));
	padded_uniform_size = get_padded_uniform_size(uniform_size);

	//Static descriptors
	static_descriptor_pool = {device, {{vk::DescriptorType::eUniformBufferDynamic, 1},
		{vk::DescriptorType::eUniformBuffer, 7}, {vk::DescriptorType::eCombinedImageSampler, 17}}};
	uniform_descriptor_set = {device, static_descriptor_pool,
		{{vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex}}};
	texture_descriptor_set = {device, static_descriptor_pool,
		{{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}};

	create_uniform_buffer();
	create_descriptors();

	//Call renderers to initialize
	Main_Renderer::initialize();
}

uint32_t Oreginum::Renderer_Core::add(Renderer_Type renderer_type, Renderable *renderable)
{
	::renderables.emplace(Key{renderer_type, ++id}, renderable);
	rerecord = true;
	return id;
}

void Oreginum::Renderer_Core::remove(Renderer_Type renderer_type, uint32_t id)
{ renderables.erase(Key{renderer_type, id}), rerecord = true; }

void Oreginum::Renderer_Core::clear(){ renderables.clear(), rerecord = true; }

void Oreginum::Renderer_Core::create_uniform_buffer()
{
	if(!renderables.empty())
	{
		uniform_buffer_size = static_cast<uint32_t>(renderables.size())*padded_uniform_size;
		uniform_buffer = {device, temporary_command_buffer, vk::BufferUsageFlagBits::eUniformBuffer,
			uniform_buffer_size, nullptr, padded_uniform_size};
	}
}

void Oreginum::Renderer_Core::create_descriptors()
{
	//Create descriptor pool
	uint32_t image_samplers{};
	auto iterator{renderables.begin()};
	while(iterator != renderables.end())
	{ image_samplers += iterator->second->get_images(); ++iterator; }
	if(image_samplers) descriptor_pool = {device,
		{{vk::DescriptorType::eCombinedImageSampler, image_samplers}}};

	//Initialize and write descriptors
	iterator = renderables.begin();
	while(iterator != renderables.end()){ iterator->second->initialize_descriptor(); ++iterator; }
	if(!renderables.empty()) uniform_descriptor_set.write({&uniform_buffer});
}

void Oreginum::Renderer_Core::record()
{
	device->get().waitIdle();

	rerecord = false;
	create_uniform_buffer();
	create_descriptors();

	//Call renderers to record command buffers
	Main_Renderer::record();
}

void Oreginum::Renderer_Core::update()
{
	//Handle window resizing
	if(Window::is_resizing())
	{
		device->get().waitIdle();
		Main_Renderer::reinitialize_swapchain();
		record();
	}

	if(Window::began_resizing())
	{
		device->get().waitIdle();
		Main_Renderer::record();
	}

	if(Window::is_resizing()) return;

	if(Window::was_resized())
	{
		device->get().waitIdle();
		Main_Renderer::create_render_passes_and_pipelines();
		Main_Renderer::create_images_and_framebuffers();
		Main_Renderer::write_descriptor_sets();
	}

	if(Window::was_resized() || rerecord) record();

	//Update the uniform buffer
	if(!renderables.empty())
	{
		void *buffer{std::malloc(uniform_buffer_size)};
		uint32_t count{};
		auto iterator{renderables.begin()};
		while(iterator != renderables.end())
		{
			iterator->second->update();
			std::memcpy(static_cast<char *>(buffer)+count*padded_uniform_size,
				iterator->second->get_uniforms(), iterator->second->get_uniforms_size());
			++iterator, ++count;
		}
		uniform_buffer.write(buffer, uniform_buffer_size);
		std::free(buffer);
	}

	Main_Renderer::update_uniforms();
}

std::shared_ptr<Oreginum::Vulkan::Instance> Oreginum::Renderer_Core::get_instance()
{ return instance; }
std::shared_ptr<Oreginum::Vulkan::Surface> Oreginum::Renderer_Core::get_surface(){ return surface; }
std::shared_ptr<Oreginum::Vulkan::Device> Oreginum::Renderer_Core::get_device(){ return device; };
const Oreginum::Vulkan::Command_Pool& Oreginum::Renderer_Core::get_command_pool()
{ return command_pool; }
const Oreginum::Vulkan::Descriptor_Pool& Oreginum::Renderer_Core::get_descriptor_pool()
{ return descriptor_pool; }
const Oreginum::Vulkan::Descriptor_Pool& Oreginum::Renderer_Core::get_static_descriptor_pool()
{ return static_descriptor_pool; }
const std::map<Oreginum::Renderer_Core::Key, Oreginum::Renderable *>&
	Oreginum::Renderer_Core::get_renderables(){ return renderables; }
uint32_t Oreginum::Renderer_Core::get_padded_uniform_size(){ return padded_uniform_size; }
Oreginum::Vulkan::Descriptor_Set Oreginum::Renderer_Core::get_uniform_descriptor_set()
{ return uniform_descriptor_set; }
Oreginum::Vulkan::Descriptor_Set Oreginum::Renderer_Core::get_texture_descriptor_set()
{ return texture_descriptor_set; }
const Oreginum::Vulkan::Command_Buffer& Oreginum::Renderer_Core::get_temporary_command_buffer()
{ return temporary_command_buffer; }
const Oreginum::Vulkan::Command_Pool& Oreginum::Renderer_Core::get_temporary_command_pool()
{ return temporary_command_pool; }
std::mutex *Oreginum::Renderer_Core::get_render_mutex(){ return &render_mutex; }