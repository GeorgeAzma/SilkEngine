#pragma once

struct GLFWmonitor;

class Monitor
{
public:
	static void init();

	Monitor(GLFWmonitor* monitor)
		: monitor(monitor) {}

	const char* getName() const;
	uvec2 getPhysicalSize() const;
	ivec2 getPosition() const;
	ivec4 getWorkArea() const;
	vec2 getContentScale() const;
	GLFWmonitor* getGLFWMonitor() const { return monitor; }

	static Monitor getPrimary();
	static std::vector<Monitor> getMonitors();

private:
	GLFWmonitor* monitor;
};