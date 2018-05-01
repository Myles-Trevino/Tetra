#pragma once
#include <GLM/glm.hpp>
#include <Windows.h>

namespace Oreginum
{
	enum Button{LEFT_MOUSE = VK_LBUTTON, RIGHT_MOUSE = VK_RBUTTON, MIDDLE_MOUSE = VK_MBUTTON};

	namespace Mouse
	{
		void update();
		void set_pressed(Button button, bool pressed = true);
		void set_locked(bool locked);
		void add_scroll_delta(int32_t scroll_delta);

		void initialize();
		void destroy();

		glm::ivec2 get_position();
		const glm::ivec2& get_delta();
		int32_t get_scroll_delta();
		bool is_locked();
		bool was_pressed(Button button);
		bool is_held(Button button);
	}
}