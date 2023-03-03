#include "fence.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Fence::Fence(bool signaled) 
	: signaled(signaled)
{
	fence = Graphics::logical_device->createFence(signaled);
}

Fence::~Fence()
{
	Graphics::logical_device->destroyFence(fence);
}

Fence::State Fence::getState() const
{
	if (signaled)
		return State::SIGNALED;
	State state = (State)Graphics::logical_device->getFenceStatus(fence);
	signaled = (state == State::SIGNALED);
	return state;
}

void Fence::reset() const
{
	if (!signaled)
		return;
	Graphics::logical_device->resetFences({ fence });
	signaled = false;
}

void Fence::wait() const
{
	if (signaled)
		return;
	Graphics::logical_device->waitForFences({ fence });
	signaled = true;
}
