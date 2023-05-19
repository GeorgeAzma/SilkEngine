#pragma once

/*
TODO:
	when window switches clear key_pressed and mouse_pressed
	have onWindowSwitchEvent?
	same for joystick
*/

enum class Key : int;
enum class MouseButton : int;

class Joystick;
enum class JoystickButton : uint8_t;
enum class JoystickHat : uint8_t;
enum class GamepadButton : uint8_t;
enum class GamepadAxis : uint8_t;

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
	
	static bool isKeyHeld(Key key);
	static bool isKeyPressed(Key key);
	static bool isKeyReleased(Key key);

	static bool isMouseHeld(MouseButton button);
	static bool isMousePressed(MouseButton button);
	static bool isMouseReleased(MouseButton button);

	static Joystick& getActiveJoystick();

	static float getJoystickAxis(int axis);
	static float getJoystickAxisDelta(int axis);

	static bool isJoystickHeld(JoystickButton button);
	static bool isJoystickPressed(JoystickButton button);
	static bool isJoystickReleased(JoystickButton button);

	static bool isJoystickHatHeld(JoystickHat button, int hat = 0);
	static bool getJoystickHatPressed(JoystickHat button, int hat = 0);
	static bool getJoystickHatReleased(JoystickHat button, int hat = 0);
	static ivec2 getJoystickHatDirection(int hat = 0);

	static float getGamepadAxis(GamepadAxis axis);
	static float getGamepadAxisDelta(GamepadAxis axis);

	static bool isGamepadHeld(GamepadButton button);
	static bool isGamepadPressed(GamepadButton button);
	static bool isGamepadReleased(GamepadButton button);

private:
	static inline struct Bindings
	{
		std::unordered_map<std::string_view, int> keys;
		std::unordered_map<std::string_view, int> mouse_buttons;
		std::unordered_map<std::string_view, int> joystick_buttons;
	} bindings;
};