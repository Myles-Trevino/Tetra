#include "../Oreginum/Core.hpp"
#include "Semaphore.hpp"

Oreginum::Vulkan::Semaphore::Semaphore(std::shared_ptr<Device> device) : device(device)
{
	vk::SemaphoreCreateInfo semaphore_information;
	if(device->get().createSemaphore(&semaphore_information, nullptr, semaphore.get()) != 
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan semaphore.");
}

Oreginum::Vulkan::Semaphore::~Semaphore()
{
	if(semaphore.use_count() != 1 || !*semaphore) return;
	device->get().destroySemaphore(*semaphore);
}

void Oreginum::Vulkan::Semaphore::swap(Semaphore *other)
{
	std::swap(device, other->device);
	std::swap(semaphore, other->semaphore);
}