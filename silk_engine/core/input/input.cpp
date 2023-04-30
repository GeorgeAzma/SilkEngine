#include "input.h"
#include "joystick.h"
#include "gfx/window/window.h"

void Input::init()
{
    Joystick::init();
}

void Input::update()
{
    Window::getActive().update();
    Joystick::getActive().update();
}

std::string Input::getClipboardString()
{
    return glfwGetClipboardString(NULL);
}

void Input::setClipboardString(std::string_view str)
{
    glfwSetClipboardString(NULL, str.data());
}

bool Input::isMouseHeld(int button) { return Window::getActive().isMouseHeld(button); }
bool Input::isMousePressed(int button) { return Window::getActive().isMousePressed(button); }
bool Input::isMouseReleased(int button) { return Window::getActive().isMouseReleased(button); }

bool Input::isKeyHeld(int key) { return Window::getActive().isKeyHeld(key); }
bool Input::isKeyPressed(int key) { return Window::getActive().isKeyPressed(key); }
bool Input::isKeyReleased(int key) { return Window::getActive().isKeyReleased(key); }

bool Input::isJoystickHeld(int button) { return Joystick::getActive().isHeld(button); }
bool Input::isJoystickPressed(int button) { return Joystick::getActive().isHeld(button); }
bool Input::isJoystickReleased(int button) { return Joystick::getActive().isHeld(button); }