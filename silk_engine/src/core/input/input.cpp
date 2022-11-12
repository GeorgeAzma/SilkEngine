#include "input.h"
#include "core/event.h"
#include <GLFW/glfw3.h>

void Input::init()
{
    Dispatcher::subscribe(onJoystickConnect);
}

std::string Input::getClipboardString()
{
    return glfwGetClipboardString(NULL);
}

InputDevice Input::getActiveInputDevice()
{
    return active_joystick ? InputDevice::JOYSTICK : InputDevice::KEYBOARD_AND_MOUSE;
}

const Input::Joystick* Input::getActiveJoystick()
{
    return active_joystick;
}

void Input::setClipboardString(std::string_view str)
{
    glfwSetClipboardString(NULL, str.data());
}

void Input::onJoystickConnect(const JoystickEvent& e)
{
    if (e.connected)
    {
        Joystick joy_stick{};
        joy_stick.id = e.id;
        joy_stick.name = glfwGetJoystickName(e.id);
        joysticks.emplace_back(std::move(joy_stick));
        active_joystick = &joysticks.back();
        SK_INFO("Joystick connected");
    }
    else
    {
        if (active_joystick && active_joystick->id == e.id)
            active_joystick = nullptr;
        for (size_t i = 0; i < joysticks.size(); ++i)
        {
            if (joysticks[i].id == e.id)
            {
                joysticks.erase(joysticks.begin() + i);
                break;
            }
        }
        SK_INFO("Joystick disconnected");
    }
}
