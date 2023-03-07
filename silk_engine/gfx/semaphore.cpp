#include "semaphore.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Semaphore::Semaphore()
{
	semaphore = Graphics::logical_device->createSemaphore();
}

Semaphore::~Semaphore()
{
	Graphics::logical_device->destroySemaphore(semaphore);
}

void Semaphore::signal(uint64_t value) const
{
	Graphics::logical_device->signalSemaphore(semaphore, value);
}
