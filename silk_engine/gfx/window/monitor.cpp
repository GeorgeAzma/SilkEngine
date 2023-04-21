#include "monitor.h"
#include "core/event.h"

void Monitor::init()
{
	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event)
		{
			bool connected = event == GLFW_CONNECTED;
			Monitor mon(monitor);
			Dispatcher<MonitorEvent>::post(mon, connected);
			SK_TRACE("Monitor {}connected: {}", connected ? "" : "dis", mon.getName());
		});
}

const char* Monitor::getName() const
{
	return glfwGetMonitorName(monitor);
}

uvec2 Monitor::getPhysicalSize() const
{
	ivec2 size;
	glfwGetMonitorPhysicalSize(monitor, &size.x, &size.y);
	return size;
}

ivec2 Monitor::getPosition() const
{
	ivec2 pos;
	glfwGetMonitorPos(monitor, &pos.x, &pos.y);
	return pos;
}

ivec4 Monitor::getWorkArea() const
{
	ivec4 work_area;
	glfwGetMonitorWorkarea(monitor, &work_area.x, &work_area.y, &work_area.z, &work_area.w);
	return work_area;
}

vec2 Monitor::getContentScale() const
{
	vec2 content_scale;
	glfwGetMonitorContentScale(monitor, &content_scale.x, &content_scale.y);
	return content_scale;
}

Monitor Monitor::getPrimary()
{
	return Monitor(glfwGetPrimaryMonitor());
}

std::vector<Monitor> Monitor::getMonitors()
{
	int count;
	auto glfw_monitors = glfwGetMonitors(&count);
	std::vector<Monitor> monitors(count, Monitor(nullptr));
	for (int i = 0; i < count; ++i)
		monitors[i] = Monitor(glfw_monitors[i]);
	return monitors;
}
