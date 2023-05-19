#include "joystick.h"
#include "core/event.h"

Joystick Joystick::active_joystick = Joystick(-1);

void Joystick::init()
{
    for (int joystick_id = GLFW_JOYSTICK_1; joystick_id <= GLFW_JOYSTICK_LAST; ++joystick_id)
        if (glfwJoystickPresent(joystick_id))
        {
            Joystick joystick(joystick_id);
            if (!active_joystick)
                active_joystick = joystick;
            connected_joysticks.emplace(joystick_id, joystick);
        }
    
    glfwSetJoystickCallback(
        [](int id, int event)
        {
            if (event == GLFW_CONNECTED)
            {
                Joystick joystick(id);
                active_joystick = joystick;
                connected_joysticks.emplace(joystick, joystick);
                Dispatcher<JoystickEvent>::post(joystick, true);
                SK_INFO("{} connected: {} [{}]", joystick.isGamepad() ? "Gamepad" : "Joystick", joystick.isGamepad() ? joystick.getGamepadName() : joystick.getName(), joystick.getGUID());
            }
            else if (event == GLFW_DISCONNECTED)
            {
                auto& disconnecting_joystick = connected_joysticks.at(id);
                Dispatcher<JoystickEvent>::post(disconnecting_joystick, false);
                SK_INFO("{} disconnected: {} [{}]", disconnecting_joystick.isGamepad() ? "Gamepad" : "Joystick", disconnecting_joystick.isGamepad() ? disconnecting_joystick.getGamepadName() : disconnecting_joystick.getName(), disconnecting_joystick.getGUID());

                connected_joysticks.erase(disconnecting_joystick);
                if (disconnecting_joystick == active_joystick)
                    active_joystick = connected_joysticks.size() ? connected_joysticks.begin()->second : Joystick(-1);
            }
        });

    if (connected_joysticks.size())
        SK_INFO("Connected joysticks:");
    for (auto&& [id, joystick] : connected_joysticks)
    {
        SK_INFO("{}: {} [{}]", joystick.isGamepad() ? "Gamepad" : "Joystick", joystick.isGamepad() ? joystick.getGamepadName() : joystick.getName(), joystick.getGUID());
    }
}

Joystick::Joystick(int id)
    : id(id)
{
    if (id == -1)
        return;

    const char* joystick_name = glfwGetJoystickName(id);
    name = joystick_name ? joystick_name : "";
    const char* joystick_guid = glfwGetJoystickGUID(id);
    guid = joystick_guid ? joystick_guid : "";
    is_gamepad = glfwJoystickIsGamepad(id) == GLFW_TRUE;
    if (is_gamepad)
    {
        const char* joystick_gamepad_name = glfwGetGamepadName(id);
        gamepad_name = joystick_gamepad_name ? joystick_gamepad_name : "";
        gamepad_buttons.resize(MAX_GAMEPAD_BUTTONS, GLFW_RELEASE);
        last_gamepad_buttons.resize(MAX_GAMEPAD_BUTTONS, GLFW_RELEASE);
        last_gamepad_axes.resize(MAX_GAMEPAD_AXIS, 0.0f);
        gamepad_axes.resize(MAX_GAMEPAD_AXIS, 0.0f);
    }
   
    int count = 0;
    glfwGetJoystickAxes(id, &count);
    last_axes.resize(count, 0.0f);
    axes.resize(count, 0.0f);
    glfwGetJoystickButtons(id, &count);
    last_buttons.resize(count, GLFW_RELEASE);
    buttons.resize(count, GLFW_RELEASE);
    glfwGetJoystickHats(id, &count);
    last_hats.resize(count, JoystickHat::CENTERED);
    hats.resize(count, JoystickHat::CENTERED);
}

void Joystick::update()
{
    if (id == -1)
        return;
    
    int count = 0;
    const float* axes_data = glfwGetJoystickAxes(id, &count);
    memcpy(last_axes.data(), axes.data(), axes.size() * sizeof(float));
    memcpy(axes.data(), axes_data, count * sizeof(float));
    for (int axis = 0; axis < count; ++axis)
    {
        if (getAxisDelta(axis) != 0.0f)
            Dispatcher<JoystickAxisEvent>::post(*this, axis, getAxis(axis), getAxisDelta(axis));
    }
    
    const uint8_t* button_data = glfwGetJoystickButtons(id, &count);
    memcpy(last_buttons.data(), buttons.data(), buttons.size());
    memcpy(buttons.data(), button_data, count * sizeof(uint8_t));
    for (int button = 0; button < count; ++button)
    {
        JoystickButton joystick_button = JoystickButton(button);
        if (isPressed(joystick_button))
            Dispatcher<JoystickPressEvent>::post(*this, joystick_button);
        else if (isReleased(joystick_button))
            Dispatcher<JoystickReleaseEvent>::post(*this, joystick_button);
    }
    
    const uint8_t* hat_data = glfwGetJoystickHats(id, &count);
    memcpy(last_hats.data(), hats.data(), hats.size());
    memcpy(hats.data(), hat_data, count * sizeof(uint8_t));
    for (int hat = 0; hat < count; ++hat)
        if (hats[hat] != last_hats[hat])
            Dispatcher<JoystickHatEvent>::post(*this, hat, getHatDirection(hat));
    
    if (is_gamepad)
    {
        GLFWgamepadstate gamepad_state;
        glfwGetGamepadState(id, &gamepad_state);
        memcpy(last_gamepad_axes.data(), gamepad_axes.data(), gamepad_axes.size() * sizeof(float));
        memcpy(gamepad_axes.data(), gamepad_state.axes, sizeof(gamepad_state.axes));
        for (int axis = 0; axis < MAX_GAMEPAD_AXIS; ++axis)
        {
            GamepadAxis gamepad_axis = GamepadAxis(axis);
            if (getGamepadAxisDelta(gamepad_axis) != 0.0f)
                Dispatcher<GamepadAxisEvent>::post(*this, gamepad_axis, getGamepadAxis(gamepad_axis), getGamepadAxisDelta(gamepad_axis));
        }
        memcpy(last_gamepad_buttons.data(), gamepad_buttons.data(), gamepad_buttons.size() * sizeof(uint8_t));
        memcpy(gamepad_buttons.data(), gamepad_state.buttons, sizeof(gamepad_state.buttons));
        for (int button = 0; button < MAX_GAMEPAD_BUTTONS; ++button)
        {
            GamepadButton gamepad_button = GamepadButton(button);
            if (isGamepadPressed(gamepad_button))
                Dispatcher<GamepadPressEvent>::post(*this, gamepad_button);
            else if (isGamepadReleased(gamepad_button))
                Dispatcher<GamepadReleaseEvent>::post(*this, gamepad_button);
        }
    }
}

bool Joystick::isPresent() const
{
    return glfwJoystickPresent(id) == GLFW_TRUE;
}
