#pragma once
#include <GLM/glm.hpp>
#include "../Vulkan/Pipeline.hpp"
#include "Renderer Core.hpp"

namespace Oreginum
{
	class Renderable
	{
	public:
		struct Uniforms{ glm::fmat4 model, view, projection; };
		enum Type{PRIMITIVE_2D, SPRITE, TEXT, PRIMITIVE_3D, ENVIRONMENT, MODEL, BUFFER,
			VOXEL, VOXEL_TRANSLUCENT};

		Renderable(){}
		~Renderable(){ remove(); }

		virtual void add(Renderer_Core::Renderer_Type renderer =
			Oreginum::Renderer_Core::Renderer_Type::MAIN);
		virtual void remove(Renderer_Core::Renderer_Type renderer =
			Oreginum::Renderer_Core::Renderer_Type::MAIN);
		virtual void initialize_descriptor(){};
		virtual const Oreginum::Vulkan::Descriptor_Set& get_descriptor_set() = 0;
		virtual void draw(const Vulkan::Command_Buffer& command_buffer){};
		virtual void update() = 0;

		virtual const void* get_uniforms() const = 0;
		virtual const uint32_t get_uniforms_size() const { return sizeof(Uniforms); };
		virtual uint8_t get_type() const { return PRIMITIVE_2D; }
		virtual uint32_t get_images() const { return 0; }

	protected:
		uint32_t id = -1;
	};
}