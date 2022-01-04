#pragma once
#include "instance.h"
#include "surface.h"

class Graphics
{
public:
	static void init(GLFWwindow* window);
	static void cleanup();
private:
	static Instance* instance;
	static Surface* surface;
};