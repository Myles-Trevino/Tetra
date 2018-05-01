#include "Core.hpp"
#include "Window.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

namespace
{
	const glm::uvec2 MINIMUM_RESOLUTION{30};

	std::string title;
	HINSTANCE instance;
	FILE *stream;
	HWND window;
	bool moving, began_resizing, resizing, resized, closed, focused;
	glm::uvec2 resolution, old_resolution, position;

	LRESULT CALLBACK window_callback(HWND window, UINT message,
		WPARAM message_information, LPARAM message_informaton_long)
	{
		switch(message)
		{
		//Keyboard
		case WM_KEYDOWN: Oreginum::Keyboard::set_pressed(
			static_cast<Oreginum::Key>(message_information)); break;

		//Mouse
		case WM_LBUTTONDOWN: Oreginum::Mouse::set_pressed(Oreginum::Button::LEFT_MOUSE); break;
		case WM_RBUTTONDOWN: Oreginum::Mouse::set_pressed(Oreginum::Button::RIGHT_MOUSE); break;
		case WM_MBUTTONDOWN: Oreginum::Mouse::set_pressed(Oreginum::Button::MIDDLE_MOUSE); break;
		case WM_MOUSEWHEEL: Oreginum::Mouse::add_scroll_delta(
				GET_WHEEL_DELTA_WPARAM(message_information)/WHEEL_DELTA); break;

		//Window
		case WM_SETFOCUS: focused = true; break;
		case WM_KILLFOCUS: focused = false; break;
		case WM_CLOSE: closed = true; break;
		default: return DefWindowProc(window, message,
			message_information, message_informaton_long);
		}
		return NULL;
	}
}

void Oreginum::Window::initialize(const std::string& title, const glm::ivec2& resolution, bool debug)
{
	::title = title;
	instance = GetModuleHandle(NULL);

	if(debug)
	{
		//Create console
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen_s(&stream, "CONOUT$", "w", stdout);
	}

	//Create window
	WNDCLASSEX window_information;
	window_information.cbSize = sizeof(WNDCLASSEX);
	window_information.style = CS_HREDRAW | CS_VREDRAW;
	window_information.lpfnWndProc = window_callback;
	window_information.cbClsExtra = 0;
	window_information.cbWndExtra = 0;
	window_information.hInstance = instance;
	window_information.hIcon = LoadIcon(instance, IDI_APPLICATION);
	window_information.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_information.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	window_information.lpszMenuName = NULL;
	window_information.lpszClassName = title.c_str();
	window_information.hIconSm = LoadIcon(window_information.hInstance, IDI_APPLICATION);
	RegisterClassEx(&window_information);

	old_resolution = ::resolution = resolution;
	position = Core::get_screen_resolution()/2-resolution/2;
	window = CreateWindow(title.c_str(), title.c_str(), WS_POPUP | WS_VISIBLE,
		position.x, position.y, resolution.x, resolution.y, NULL, NULL, instance, NULL);
	if(!window) Core::error("Could not create window.");
}

void Oreginum::Window::destroy()
{
	DestroyWindow(window);
	//FreeConsole();
}

void Oreginum::Window::update()
{
	//Reset
	resized = false;
	moving = false;

	//Dispatch messages to callback
	static MSG message;
	while(PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE)) DispatchMessage(&message);

	if(Keyboard::is_held(Key::ESC)) closed = true;

	//Move
	if(Mouse::is_held(Button::LEFT_MOUSE) && Keyboard::is_held(CTRL) && !Mouse::is_locked())
	{
		position += Mouse::get_delta();
		MoveWindow(window, position.x, position.y, resolution.x, resolution.y, false);
		moving = true;
	}
			
	//Resize
	else if(Mouse::is_held(Button::RIGHT_MOUSE) && Keyboard::is_held(CTRL) && !Mouse::is_locked())
	{
		resolution = {glm::clamp(glm::ivec2{resolution}+Mouse::get_delta(),
			glm::ivec2{MINIMUM_RESOLUTION}, glm::ivec2{INT32_MAX})};
		MoveWindow(window, position.x, position.y, resolution.x, resolution.y, false);

		//Set resizing booleans
		::began_resizing = Mouse::was_pressed(Button::RIGHT_MOUSE);
		resizing = true;
	} else if(resizing) resized = true, resizing = false;
}

HINSTANCE Oreginum::Window::get_instance(){ return instance; }

HWND Oreginum::Window::get(){ return window; }

const std::string& Oreginum::Window::get_title(){ return title; }

glm::uvec2 Oreginum::Window::get_resolution(){ return resolution; }

float Oreginum::Window::get_aspect_ratio()
{ return (resolution.x >= resolution.y) ? static_cast<float>(resolution.x)/resolution.y :
	static_cast<float>(resolution.y)/resolution.x; }

glm::uvec2 Oreginum::Window::get_position(){ return position; }

bool Oreginum::Window::was_closed(){ return closed; }

bool Oreginum::Window::is_moving(){ return moving; }

bool Oreginum::Window::began_resizing(){ return ::began_resizing; }

bool Oreginum::Window::is_resizing(){ return resizing; }

bool Oreginum::Window::was_resized(){ return resized; }

bool Oreginum::Window::is_visible(){ return (resolution.x > 0 && resolution.y > 0); }

bool Oreginum::Window::has_focus(){ return focused; }