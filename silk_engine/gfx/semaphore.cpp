#include "semaphore.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

Semaphore::Semaphore()
{
	semaphore = RenderContext::getLogicalDevice().createSemaphore();
}

Semaphore::~Semaphore()
{
	RenderContext::getLogicalDevice().destroySemaphore(semaphore);
}

void Semaphore::signal(uint64_t value) const
{
	RenderContext::getLogicalDevice().signalSemaphore(semaphore, value);
}
