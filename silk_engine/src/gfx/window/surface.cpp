#include "surface.h"
#include "core/event.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"
#include "gfx/devices/physical_device.h"
#include <GLFW/glfw3.h>

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, Window::getGLFWWindow(), nullptr, &surface));
}

Surface::~Surface()
{
	Graphics::instance->destroySurface(surface);
}