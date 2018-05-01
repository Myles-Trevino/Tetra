#pragma once
#include <map>
#include <mutex>
#include <tuple>
#include "../Vulkan/Pipeline.hpp"
#include "../Vulkan/Command Pool.hpp"
#include "../Vulkan/Framebuffer.hpp"
#include "../Vulkan/Semaphore.hpp"

namespace Oreginum
{
	class Renderable;

	namespace Renderer_Core
	{
		enum Renderer_Type{MAIN, GLYPH_BUFFER, BUFFER};
		class Key
		{
		public:
			Renderer_Type renderer;
			uint32_t id;
			bool operator <(const Key& a) const
			{ return std::to_string(renderer)+std::to_string(id)
				< std::to_string(a.renderer)+std::to_string(a.id); }
		};

		void initialize();
		uint32_t add(Renderer_Type renderer_type, Renderable *renderable);
		void remove(Renderer_Type renderer_type, uint32_t id);
		Vulkan::Pipeline create_pipeline(const glm::ivec2& resolution,
			const Vulkan::Render_Pass& render_pass, const std::string& vertex,
			const std::string& fragment, uint8_t render_pass_number,
			const std::vector<vk::DescriptorSetLayout>& descriptor_layouts,
			const Vulkan::Pipeline& base = {});
		void submit_command_buffers(const std::vector<vk::CommandBuffer>& command_buffers,
			const std::vector<vk::Semaphore>& wait_semaphores = {},
			const std::vector<vk::PipelineStageFlags>& wait_stages = {},
			const std::vector<vk::Semaphore>& signal_semaphores = {});
		void update();

		void clear();
		void create_uniform_buffer();
		void create_descriptors();
		void record();

		std::shared_ptr<Oreginum::Vulkan::Instance> get_instance();
		std::shared_ptr<Vulkan::Surface> get_surface();
		std::shared_ptr<Vulkan::Device> get_device();
		const Vulkan::Command_Pool& get_command_pool();
		const Vulkan::Descriptor_Pool& get_descriptor_pool();
		const Vulkan::Descriptor_Pool& get_static_descriptor_pool();
		const std::map<Oreginum::Renderer_Core::Key, Oreginum::Renderable *>& get_renderables();
		uint32_t get_padded_uniform_size();
		uint32_t get_padded_uniform_size(uint32_t uniform_size);
		Vulkan::Descriptor_Set get_uniform_descriptor_set();
		Vulkan::Descriptor_Set get_texture_descriptor_set();
		const Vulkan::Command_Buffer& get_temporary_command_buffer();
		const Vulkan::Command_Pool& get_temporary_command_pool();
        std::mutex *get_render_mutex();
	};
}