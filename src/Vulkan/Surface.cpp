#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Surface.hpp"

Oreginum::Vulkan::Surface::Surface(std::shared_ptr<Instance> instance) : instance(instance)
{
	vk::Win32SurfaceCreateInfoKHR surface_information
	{{}, Oreginum::Window::get_instance(), Oreginum::Window::get()};
	if(instance->get().createWin32SurfaceKHR(&surface_information, nullptr, surface.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan surface.");
}

Oreginum::Vulkan::Surface::~Surface()
{ if(surface.use_count() == 1 && *surface) instance->get().destroySurfaceKHR(*surface); }

void Oreginum::Vulkan::Surface::swap(Surface *other)
{
	std::swap(instance, other->instance);
	std::swap(surface, other->surface);
}