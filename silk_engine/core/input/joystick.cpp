#include "joystick.h"
#include "core/event.h"

Joystick Joystick::active_joystick = Joystick(-1);

void Joystick::init()
{
    glfwSetJoystickCallback(
        [](int id, int event)
        {
            bool connected = event == GLFW_CONNECTED;
            Joystick joystick(id);
            if (connected)
            {
                active_joystick = joystick;
                connected_joysticks.emplace(joystick, joystick);
            }
            else
            {
                connected_joysticks.erase(joystick);
                if (joystick == active_joystick)
                    active_joystick = connected_joysticks.size() ? connected_joysticks.begin()->second : Joystick(-1);
            }
            Dispatcher<JoystickEvent>::post(joystick, connected);
            SK_INFO("Joystick {}connected: {}", connected ? "" : "dis", joystick.getName());
        });
}

Joystick::Joystick(int id)
    : id(id)
{
    if (id == -1)
        return;

    name = glfwGetJoystickName(id);
    guid = glfwGetJoystickGUID(id);
    is_gamepad = glfwJoystickIsGamepad(id) == GLFW_TRUE;
    if (is_gamepad)
    {
        gamepad_name = glfwGetGamepadName(id);
        gamepad_buttons.resize(15, GLFW_RELEASE);
        last_gamepad_buttons.resize(15, GLFW_RELEASE);
        gamepad_axes.resize(6, 0.0f);
    }

    int count = 0;
    glfwGetJoystickAxes(id, &count);
    axes.resize(count, 0.0f);
    glfwGetJoystickButtons(id, &count);
    last_buttons.resize(count, GLFW_RELEASE);
    buttons.resize(count, GLFW_RELEASE);
    glfwGetJoystickHats(id, &count);
    hats.resize(count, GLFW_HAT_CENTERED);
}

void Joystick::update()
{
    if (id == -1)
        return;
    int count = 0;
    const float* axes_data = glfwGetJoystickAxes(id, &count);
    memcpy(axes.data(), axes_data, count * sizeof(float));
    const byte* button_data = glfwGetJoystickButtons(id, &count);
    memcpy(last_buttons.data(), buttons.data(), buttons.size());
    memcpy(buttons.data(), button_data, count * sizeof(byte));
    const byte* hat_data = glfwGetJoystickHats(id, &count);
    memcpy(hats.data(), hat_data, count * sizeof(byte));
    if (is_gamepad)
    {
        GLFWgamepadstate gamepad_state;
        glfwGetGamepadState(id, &gamepad_state);
        memcpy(gamepad_axes.data(), gamepad_state.axes, sizeof(gamepad_state.axes));
        memcpy(last_gamepad_buttons.data(), gamepad_buttons.data(), gamepad_buttons.size() * sizeof(byte));
        memcpy(gamepad_buttons.data(), gamepad_state.buttons, sizeof(gamepad_state.buttons));
    }
}

bool Joystick::isPresent() const
{
    return glfwJoystickPresent(id) == GLFW_TRUE;
}
