#pragma once

class GLFW
{
public:
	static void init();
	static void destroy();

private:
	static void errorCallback(int error, const char* description);
};