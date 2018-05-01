#define NOMINMAX
#include <windows.h>
#include "Core.hpp"
#include "Window.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

namespace
{
	glm::ivec2 position;
	glm::ivec2 delta;
	bool locked;
	int32_t scroll_delta{};
    bool pressed[3], first_update{true};

	glm::ivec2 get_window_center()
	{ return Oreginum::Window::get_position()+Oreginum::Window::get_resolution()/2U; }

	void center(){ SetCursorPos(get_window_center().x, get_window_center().y); }
	void lock(){ ShowCursor(false), center(), locked = true; }
	void free(){ ShowCursor(true), locked = false; }
}

void Oreginum::Mouse::set_locked(bool locked){ if(locked) lock(); else free(); }

void Oreginum::Mouse::initialize(){ lock(); }

void Oreginum::Mouse::destroy(){ free(); }

void Oreginum::Mouse::update()
{
	scroll_delta = 0;

	for(bool& b : pressed) b = false;

	if(Keyboard::was_pressed(Key::ESC)) if(locked) free(); else lock();

	//Get the new position
	static POINT point{};
	GetCursorPos(&point);
	glm::ivec2 previous_position{position};
	position = {point.x, point.y};

	//Get the delta
	if(locked)
	{
		delta = position-get_window_center();
		center();
	} else delta = position-previous_position; 

    if(first_update) delta = {}, first_update = false;
}

void Oreginum::Mouse::set_pressed(Button button, bool pressed){ ::pressed[button] = pressed; }

void Oreginum::Mouse::add_scroll_delta(int32_t scroll_delta){ ::scroll_delta += scroll_delta; }

glm::ivec2 Oreginum::Mouse::get_position()
{
	POINT position{};
	GetCursorPos(&position);
	return glm::uvec2{position.x, position.y}-Window::get_position();
}

const glm::ivec2& Oreginum::Mouse::get_delta(){ return delta; }

int32_t Oreginum::Mouse::get_scroll_delta(){ return scroll_delta; }

bool Oreginum::Mouse::is_locked(){ return locked; }

bool Oreginum::Mouse::was_pressed(Button button){ return pressed[button]; }

bool Oreginum::Mouse::is_held(Button button)
{ return (GetAsyncKeyState(button) != 0) && Window::has_focus(); }