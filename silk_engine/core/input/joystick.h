#pragma once

class Joystick
{
public:
	static void init();

public:
	Joystick(int id);

	void update();

	bool isPresent() const;

	const std::vector<float>& getAxes() const { return axes; }
	const std::vector<byte>& getButtons() const { return buttons; }
	std::string_view getGUID() const { return guid; }
	const std::vector<byte>& getHats() const { return hats; }
	std::string_view getName() const { return name; }
	bool isHeld(int button) const { return buttons[button] == GLFW_PRESS; }
	bool isPressed(int button) const { return buttons[button] == GLFW_PRESS && last_buttons[button] == GLFW_RELEASE; }
	bool isReleased(int button) const { return buttons[button] == GLFW_RELEASE && last_buttons[button] == GLFW_PRESS; }

	bool isGamepad() const { return is_gamepad; }
	const std::vector<byte>& getGamepadButtons() const { return gamepad_buttons; }
	std::string_view getGamepadName() const { return gamepad_name; }
	bool isGamepadHeld(int button) const { return gamepad_buttons[button] == GLFW_PRESS; }
	bool isGamepadPressed(int button) const { return gamepad_buttons[button] == GLFW_PRESS && last_gamepad_buttons[button] == GLFW_RELEASE; }
	bool isGamepadReleased(int button) const { return gamepad_buttons[button] == GLFW_RELEASE && last_gamepad_buttons[button] == GLFW_PRESS; }

	bool operator==(const Joystick& other) const { return id == other.id; }
	bool operator==(int id) const { return this->id == id; }
	operator int() const { return id; }
	operator bool() const { return id != -1; }

private:
	int id = -1;
	std::vector<float> axes;
	std::vector<byte> buttons;
	std::vector<byte> last_buttons;
	std::vector<byte> hats;
	std::string name;
	std::string guid;
	bool is_gamepad = false;
	std::vector<byte> gamepad_buttons;
	std::vector<byte> last_gamepad_buttons;
	std::vector<float> gamepad_axes;
	std::string gamepad_name;

public:
	static Joystick& getActive() { return active_joystick; }
	static void setActive(const Joystick& joystick) { active_joystick = joystick; }

private:
	static Joystick active_joystick;
	static inline std::unordered_map<int, Joystick> connected_joysticks;
};