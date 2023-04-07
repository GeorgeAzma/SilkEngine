#include "joystick.h"
#include "core/event.h"

void Joystick::init()
{
    glfwSetJoystickCallback(
        [](int id, int event)
        {
            bool connected = event == GLFW_CONNECTED;
            Joystick joystick(id);
            Dispatcher::post(JoystickEvent(joystick, connected));
            SK_INFO("Joystick {}connected: {}", connected ? "" : "dis", joystick.getName());
        });
}

std::span<const float> Joystick::getAxes() const
{
    int count = 0; 
    const float* data = glfwGetJoystickAxes(id, &count);
    return std::span<const float>(data, count);
}

std::span<const unsigned char> Joystick::getButtons() const
{
    int count = 0;
    const unsigned char* data = glfwGetJoystickButtons(id, &count);
    return std::span<const unsigned char>(data, count);
}

const char* Joystick::getGUID() const
{
    return glfwGetJoystickGUID(id);
}

std::span<const unsigned char> Joystick::getHats() const
{
    int count = 0;
    const unsigned char* data = glfwGetJoystickHats(id, &count);
    return std::span<const unsigned char>(data, count);
}

const char* Joystick::getName() const
{
	return glfwGetJoystickName(id);
}

bool Joystick::isGamepad() const
{
    return glfwJoystickIsGamepad(id) == GLFW_TRUE;
}

bool Joystick::isPresent() const
{
    return glfwJoystickPresent(id) == GLFW_TRUE;
}
