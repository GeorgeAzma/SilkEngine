#include "input.h"
#include "joystick.h"
#include "silk_engine/gfx/window/window.h"

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

bool Input::isKeyHeld(Key key) { return Window::get().isKeyHeld(key); }
bool Input::isKeyPressed(Key key) { return Window::get().isKeyPressed(key); }
bool Input::isKeyReleased(Key key) { return Window::get().isKeyReleased(key); }

bool Input::isMouseHeld(MouseButton button) { return Window::get().isMouseHeld(button); }
bool Input::isMousePressed(MouseButton button) { return Window::get().isMousePressed(button); }
bool Input::isMouseReleased(MouseButton button) { return Window::get().isMouseReleased(button); }

vec2 Input::getMouse() { return Window::get().getMouse(); }

Joystick& Input::getActiveJoystick() { return Joystick::getActive(); }

float Input::getJoystickAxis(int axis) { return Joystick::getActive().getAxis(axis); }
float Input::getJoystickAxisDelta(int axis) { return Joystick::getActive().getAxisDelta(axis); }

bool Input::isJoystickHeld(JoystickButton button) { return Joystick::getActive().isHeld(button); }
bool Input::isJoystickPressed(JoystickButton button) { return Joystick::getActive().isPressed(button); }
bool Input::isJoystickReleased(JoystickButton button) { return Joystick::getActive().isReleased(button); }

bool Input::isJoystickHatHeld(JoystickHat button, int hat) { return Joystick::getActive().isHatHeld(button); }
bool Input::getJoystickHatPressed(JoystickHat button, int hat) { return Joystick::getActive().isHatPressed(button); }
bool Input::getJoystickHatReleased(JoystickHat button, int hat) { return Joystick::getActive().isHatReleased(button); }
ivec2 Input::getJoystickHatDirection(int hat) { return Joystick::getActive().getHatDirection(hat); }

float Input::getGamepadAxis(GamepadAxis axis) { return Joystick::getActive().getGamepadAxis(axis); }
float Input::getGamepadAxisDelta(GamepadAxis axis) { return Joystick::getActive().getGamepadAxisDelta(axis); }

bool Input::isGamepadHeld(GamepadButton button) { return Joystick::getActive().isGamepadHeld(button); }
bool Input::isGamepadPressed(GamepadButton button) { return Joystick::getActive().isGamepadPressed(button); }
bool Input::isGamepadReleased(GamepadButton button) { return Joystick::getActive().isGamepadReleased(button); }