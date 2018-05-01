#pragma once
#include "../Oreginum/Renderable.hpp"
#include "../Oreginum/Texture.hpp"

namespace Tetra
{
	class Render_Group : public Oreginum::Renderable
	{
	public:
		Render_Group(const std::vector<float>& vertices, const std::vector<uint32_t>& indices,
			uint8_t material_type, const glm::fvec3& translation);
		void initialize_descriptor();
		uint32_t get_images() const { return 1; }
		void update();
		void draw(const Oreginum::Vulkan::Command_Buffer& command_buffer);
		void translate(const glm::fvec3& translation){ this->translation += translation; }

		const Oreginum::Vulkan::Descriptor_Set& get_descriptor_set(){ return descriptor_set; }
		const void *get_uniforms() const { return &uniforms; }
		const uint32_t get_uniforms_size() const { return sizeof(Uniforms); }
		uint8_t get_type() const { return material_type; }

	private:
		static Oreginum::Vulkan::Sampler sampler;
		static Oreginum::Vulkan::Descriptor_Set descriptor_set;
		static Oreginum::Texture texture_map;

		glm::fvec3 translation;
		Oreginum::Vulkan::Buffer vertex_buffer, index_buffer;
		uint32_t index_count;
		Uniforms uniforms;
		uint8_t material_type;
	};
}