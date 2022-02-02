#pragma once

#include "core/event.h"

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

	static void setKey(const std::string& name, int key);
	static void setMouseButton(const std::string& name, int button);
	static void setJoystickButton(const std::string& name, int button);

	static int getKey(const std::string& name);
	static int getMouseButton(const std::string& name);
	static int getJoystickButton(const std::string& name);

	static glm::vec2 getMouse();
	static float getMouseX();
	static float getMouseY();

	static std::string getClipboard();
	static void setClipboard(const std::string& str);

	static void lockMouse();
	static void unlockMouse();

	static InputDevice getActiveInputDevice();
	static const Joystick* getActiveJoystick();

	/**
	* @param hot_spot cursor's image spot where it will get clicked
	*/
	static void setCursor(const std::string& file, CursorHotSpot hot_spot = CursorHotSpot::TOP_RIGHT);

private:
	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onMouseMove(const MouseMoveEvent& e);
	static void onKeyPress(const KeyPressEvent& e);
	static void onKeyRelease(const KeyReleaseEvent& e);
	static void onJoystickConnect(const JoystickEvent& e);

private:
	static inline std::array<bool, GLFW_KEY_LAST + 1> keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> mouse_buttons{false};

	static inline std::array<bool, GLFW_KEY_LAST + 1> last_keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> last_mouse_buttons{false};

	static inline glm::vec2 mouse = glm::vec2(0);

	static inline std::unordered_map<std::string, int> key_binds;
	static inline std::unordered_map<std::string, int> mouse_button_binds;
	static inline std::unordered_map<std::string, int> joystick_button_binds;

	static inline GLFWcursor* cursor = nullptr;
	
	static inline Joystick* active_joystick = nullptr;
	static inline std::vector<Joystick> joysticks;
};