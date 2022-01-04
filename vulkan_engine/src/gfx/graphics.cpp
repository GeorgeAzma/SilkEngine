#include "graphics.h"
#include "instance.h"

void Graphics::init()
{
	VE_CORE_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");
	instance = new Instance();
}