#include "input.h"
#include "joystick.h"
#include <GLFW/glfw3.h>

void Input::init()
{
    Joystick::init();
}

std::string Input::getClipboardString()
{
    return glfwGetClipboardString(NULL);
}

void Input::setClipboardString(std::string_view str)
{
    glfwSetClipboardString(NULL, str.data());
}