#include "glfw.h"
#include <GLFW/glfw3.h>

void GLFW::init()
{
    glfwInit();
    glfwSetErrorCallback(GLFW::errorCallback);
}

void GLFW::destroy()
{
    glfwTerminate();
}

void GLFW::errorCallback(int error, const char* description)
{
    SK_ERROR("GLFW({}): {}", error, description);
}