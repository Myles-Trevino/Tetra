#pragma once
#include "../Vulkan/Swapchain.hpp"

namespace Oreginum
{
	namespace Main_Renderer
	{
		void initialize();
		void render();

		void create_render_passes_and_pipelines();
		void create_images_and_framebuffers();
		void write_descriptor_sets();
		void update_uniforms();
		void record();

		void reinitialize_swapchain();
	};
}