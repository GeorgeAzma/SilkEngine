#include "joystick.h"
#include "core/event.h"
#include <GLFW/glfw3.h>

Joystick Joystick::active_joystick = Joystick(-1);

void Joystick::init()
{
    glfwSetJoystickCallback(
        [](int id, int event)
        {
            bool connected = event == GLFW_CONNECTED;
            Joystick joystick(id);
            Dispatcher::post(JoystickEvent(joystick, connected));
            if (connected)
            {
                SK_INFO("Joystick connected: {}", joystick.getName());
                joysticks.emplace_back(joystick);
                active_joystick = joystick;

            }
            else
            {
                SK_INFO("Joystick disconnected: {}", joystick.getName());
                for (size_t i = 0; i < joysticks.size(); ++i)
                {
                    if (joysticks[i] == joystick)
                    {
                        std::swap(joysticks[i], joysticks.back());
                        joysticks.pop_back();
                        break;
                    }
                }
                if (active_joystick == joystick)
                    active_joystick = Joystick(-1);
            }
        });
}

const char* Joystick::getName()
{
	return glfwGetJoystickName(id);
}

Joystick Joystick::getActive()
{
	return active_joystick;
}
