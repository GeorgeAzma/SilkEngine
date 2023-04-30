#pragma once

/*
TODO:
	when window switches clear key_pressed and mouse_pressed
	have onWindowSwitchEvent?
	same for joystick
*/

enum class InputDevice : uint32_t
{
	KEYBOARD_AND_MOUSE,
	JOYSTICK
};

class Input
{
public:
	static void init();
	static void update();

	static void setKeyBind(std::string_view name, int key) { bindings.keys[name] = key; }
	static void setMouseButtonBind(std::string_view name, int button) { bindings.mouse_buttons[name] = button; }
	static void setJoystickButtonBind(std::string_view name, int button) { bindings.joystick_buttons[name] = button; }
	static void setClipboardString(std::string_view str);

	static int getKeyBind(std::string_view name) { return bindings.keys.at(name); }
	static int getMouseButtonBind(std::string_view name) { return bindings.mouse_buttons.at(name); }
	static int getJoystickButtonBind(std::string_view name) { return bindings.joystick_buttons.at(name); }
	static std::string getClipboardString();

	static bool isMouseHeld(int button);
	static bool isMousePressed(int button);
	static bool isMouseReleased(int button);
	
	static bool isKeyHeld(int key);
	static bool isKeyPressed(int key);
	static bool isKeyReleased(int key);

	static bool isJoystickHeld(int button);
	static bool isJoystickPressed(int button);
	static bool isJoystickReleased(int button);

private:
	static inline struct Bindings
	{
		std::unordered_map<std::string_view, int> keys;
		std::unordered_map<std::string_view, int> mouse_buttons;
		std::unordered_map<std::string_view, int> joystick_buttons;
	} bindings;
};