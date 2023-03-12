#pragma once

struct GLFWmonitor;

class Monitor
{
public:
	static void init();

	static Monitor getPrimary();
	static std::vector<Monitor> getMonitors();

	Monitor(GLFWmonitor* monitor)
		: monitor(monitor) {}

	const char* getName() const;
	uvec2 getPhysicalSize() const;
	ivec2 getPosition() const;
	ivec4 getWorkArea() const;
	vec2 getContentScale() const;

	operator GLFWmonitor*() const { return monitor; }
	operator const GLFWmonitor*() const { return monitor; }

private:
	GLFWmonitor* monitor = nullptr;
};