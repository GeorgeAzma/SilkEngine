#include "fence.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

Fence::Fence(bool signaled) 
	: signaled(signaled)
{
	fence = RenderContext::getLogicalDevice().createFence(signaled);
}

Fence::~Fence()
{
	RenderContext::getLogicalDevice().destroyFence(fence);
}

Fence::State Fence::getState() const
{
	if (signaled)
		return State::SIGNALED;
	State state = (State)RenderContext::getLogicalDevice().getFenceStatus(fence);
	signaled = (state == State::SIGNALED);
	return state;
}

void Fence::reset() const
{
	if (!signaled)
		return;
	RenderContext::getLogicalDevice().resetFences({ fence });
	signaled = false;
}

void Fence::wait() const
{
	if (signaled)
		return;
	RenderContext::getLogicalDevice().waitForFences({ fence });
	signaled = true;
}
