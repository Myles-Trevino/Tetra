#pragma once
#include <string>
#define NOMINMAX
#include <windows.h>
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Window
	{
		void initialize(const std::string& title,
			const glm::ivec2& resolution, bool debug = false);
		void destroy();

		void update();

		HINSTANCE get_instance();
		HWND get();
		const std::string& get_title();
		glm::uvec2 get_resolution();
		float get_aspect_ratio();
		glm::uvec2 get_position();
		bool was_closed();
		bool is_moving();
		bool began_resizing();
		bool is_resizing();
		bool was_resized();
		bool is_visible();
		bool has_focus();
	};
}