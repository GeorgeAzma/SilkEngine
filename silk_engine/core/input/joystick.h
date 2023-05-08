#pragma once

enum class JoystickButton : byte
{
	_1 = 0,
	_2 = 1,
	_3 = 2,
	_4 = 3,
	_5 = 4,
	_6 = 5,
	_7 = 6,
	_8 = 7,
	_9 = 8,
	_10 = 9,
	_11 = 10,
	_12 = 11,
	_13 = 12,
	_14 = 13,
	_15 = 14,
	_16 = 15,
	LAST = _16
};

enum class JoystickHat : byte
{
	CENTERED = 0,
	UP = 1,
	RIGHT = 2,
	DOWN = 4,
	LEFT = 8,

	RIGHT_UP = (RIGHT | UP),
	RIGHT_DOWN = (RIGHT | DOWN),
	LEFT_UP = (LEFT | UP),
	LEFT_DOWN = (LEFT | DOWN)
};

static JoystickHat operator&(JoystickHat lhs, JoystickHat rhs)
{
	return JoystickHat(ecast(lhs) & ecast(rhs));
}

enum class GamepadButton : byte
{
	A = 0,
	B = 1,
	X = 2,
	Y = 3,
	LEFT_BUMPER = 4,
	RIGHT_BUMPER = 5,
	BACK = 6,
	START = 7,
	GUIDE = 8,
	LEFT_THUMB = 9,
	RIGHT_THUMB = 10,
	DPAD_UP = 11,
	DPAD_RIGHT = 12,
	DPAD_DOWN = 13,
	DPAD_LEFT = 14,
	LAST = DPAD_LEFT,

	CROSS = A,
	CIRCLE = B,
	SQUARE = X,
	TRIANGLE = Y
};

enum class GamepadAxis : byte
{
	LEFT_X = 0,
	LEFT_Y = 1,
	RIGHT_X = 2,
	RIGHT_Y = 3,
	LEFT_TRIGGER = 4,
	RIGHT_TRIGGER = 5,
	LAST = RIGHT_TRIGGER
};

class Joystick
{
public:
	static constexpr int MAX_GAMEPAD_AXIS = 6;
	static constexpr int MAX_GAMEPAD_BUTTONS = 15;

public:
	static void init();

public:
	Joystick(int id);

	void update();

	const std::vector<float>& getAxes() const { return axes; }
	float getAxis(int axis) const { return axes[axis]; }
	float getAxisDelta(int axis) const { return axes[axis] - last_axes[axis]; }
	
	bool isHeld(JoystickButton button) const { return buttons[ecast(button)] == GLFW_PRESS; }
	bool isPressed(JoystickButton button) const { return buttons[ecast(button)] == GLFW_PRESS && last_buttons[ecast(button)] == GLFW_RELEASE; }
	bool isReleased(JoystickButton button) const { return buttons[ecast(button)] == GLFW_RELEASE && last_buttons[ecast(button)] == GLFW_PRESS; }

	bool isHatHeld(JoystickHat button, int hat = 0) const { return (hats[hat] & button) == button; }
	bool isHatPressed(JoystickHat button, int hat = 0) const { return (hats[hat] & button) == button && (last_hats[hat] & button) == JoystickHat(0); }
	bool isHatReleased(JoystickHat button, int hat = 0) const { return (hats[hat] & button) == JoystickHat(0) && (last_hats[hat] & button) == button; }
	ivec2 getHatDirection(int hat = 0) const 
	{ 
		return ((ecast(hats[hat]) & GLFW_HAT_UP   ) > 0 ? ivec2( 0,  1) : ivec2(0)) +
			   ((ecast(hats[hat]) & GLFW_HAT_RIGHT) > 0 ? ivec2( 1,  0) : ivec2(0)) +
			   ((ecast(hats[hat]) & GLFW_HAT_DOWN ) > 0 ? ivec2( 0, -1) : ivec2(0)) +
			   ((ecast(hats[hat]) & GLFW_HAT_LEFT ) > 0 ? ivec2(-1,  0) : ivec2(0));
	}
	
	std::string_view getName() const { return name; }
	std::string_view getGUID() const { return guid; }
	
	bool isGamepad() const { return is_gamepad; }

	const std::vector<float>& getGamepadAxes() const { return gamepad_axes; }
	float getGamepadAxis(GamepadAxis axis) const { return gamepad_axes[ecast(axis)]; }
	float getGamepadAxisDelta(GamepadAxis axis) const { return gamepad_axes[ecast(axis)] - last_gamepad_axes[ecast(axis)]; }

	bool isGamepadHeld(GamepadButton button) const { return gamepad_buttons[ecast(button)] == GLFW_PRESS; }
	bool isGamepadPressed(GamepadButton button) const { return gamepad_buttons[ecast(button)] == GLFW_PRESS && last_gamepad_buttons[ecast(button)] == GLFW_RELEASE; }
	bool isGamepadReleased(GamepadButton button) const { return gamepad_buttons[ecast(button)] == GLFW_RELEASE && last_gamepad_buttons[ecast(button)] == GLFW_PRESS; }
	
	std::string_view getGamepadName() const { return gamepad_name; }

	bool isPresent() const;

	bool operator==(const Joystick& other) const { return id == other.id; }
	bool operator==(int id) const { return this->id == id; }
	operator int() const { return id; }
	operator bool() const { return id != -1; }

private:
	int id = -1;
	std::vector<float> axes;
	std::vector<float> last_axes;
	std::vector<byte> buttons;
	std::vector<byte> last_buttons;
	std::vector<JoystickHat> hats;
	std::vector<JoystickHat> last_hats;
	std::string name = "";
	std::string guid = "";
	bool is_gamepad = false;
	std::vector<byte> gamepad_buttons;
	std::vector<byte> last_gamepad_buttons;
	std::vector<float> gamepad_axes;
	std::vector<float> last_gamepad_axes;
	std::string gamepad_name = "";

public:
	static Joystick& getActive() { return active_joystick; }
	static void setActive(const Joystick& joystick) { active_joystick = joystick; }

private:
	static Joystick active_joystick;
	static inline std::unordered_map<int, Joystick> connected_joysticks;
};