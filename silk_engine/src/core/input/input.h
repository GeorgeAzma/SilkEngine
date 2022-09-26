#pragma once

#include "core/event.h"

struct GLFWcursor;

enum class CursorHotSpot
{
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	CENTER
};

enum class InputDevice : uint32_t
{
	KEYBOARD_AND_MOUSE,
	JOYSTICK
};

class Input
{
	struct Joystick
	{
		int id;
		std::string name;
	};
public:
	static void init();

	static void update();

	static bool isMouseDown(int button);
	static bool isKeyDown(int key);
	static bool isMousePressed(int button);
	static bool isKeyPressed(int key);
	static bool isMouseReleased(int button);
	static bool isKeyReleased(int key);

	static void setKey(std::string_view name, int key);
	static void setMouseButton(std::string_view name, int button);
	static void setJoystickButton(std::string_view name, int button);

	static int getKey(std::string_view name);
	static int getMouseButton(std::string_view name);
	static int getJoystickButton(std::string_view name);

	static vec2 getMouse();
	static float getMouseX();
	static float getMouseY();

	static void lockMouse();
	static void unlockMouse();

	static void setClipboardString(std::string_view str);
	static std::string getClipboardString();

	static InputDevice getActiveInputDevice();
	static const Joystick* getActiveJoystick();

	/**
	* @param hot_spot cursor's image spot where it will get clicked
	*/
	static void setCursor(std::string_view file, CursorHotSpot hot_spot = CursorHotSpot::TOP_RIGHT);

private:
	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onMouseMove(const MouseMoveEvent& e);
	static void onKeyPress(const KeyPressEvent& e);
	static void onKeyRelease(const KeyReleaseEvent& e);
	static void onJoystickConnect(const JoystickEvent& e);

private:
	static inline std::vector<uint8_t> keys;
	static inline std::vector<uint8_t> last_keys;
	static inline std::vector<uint8_t> mouse_buttons;
	static inline std::vector<uint8_t> last_mouse_buttons;

	static inline vec2 mouse = vec2(0);

	static inline std::unordered_map<std::string_view, int> key_binds;
	static inline std::unordered_map<std::string_view, int> mouse_button_binds;
	static inline std::unordered_map<std::string_view, int> joystick_button_binds;

	static inline GLFWcursor* cursor = nullptr;
	
	static inline Joystick* active_joystick = nullptr;
	static inline std::vector<Joystick> joysticks;
};