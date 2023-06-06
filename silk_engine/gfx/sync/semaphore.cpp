#include "semaphore.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"

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

void Semaphore::wait(uint64_t value, uint64_t timeout) const
{
	RenderContext::getLogicalDevice().waitForSemaphores({ semaphore }, { value }, 0, timeout);
}
