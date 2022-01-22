#pragma once

#include "core/event.h"

class Input
{
public:
	static void init();

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

	static void lockMouse();
	static void unlockMouse();

private:
	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onMouseMove(const MouseMoveEvent& e);
	static void onKeyPress(const KeyPressEvent& e);
	static void onKeyRelease(const KeyReleaseEvent& e);

private:
	static inline std::array<bool, GLFW_KEY_LAST + 1> keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> mouse_buttons{false};

	static inline std::array<bool, GLFW_KEY_LAST + 1> last_keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> last_mouse_buttons{false};

	static inline glm::vec2 mouse = glm::vec2(0);

	static inline std::unordered_map<std::string, int> key_binds;
	static inline std::unordered_map<std::string, int> mouse_button_binds;
	static inline std::unordered_map<std::string, int> joystick_button_binds;
};