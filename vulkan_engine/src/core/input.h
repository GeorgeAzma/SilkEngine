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

private:
	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onKeyPress(const KeyPressEvent& e);
	static void onKeyRelease(const KeyReleaseEvent& e);

private:
	static inline std::array<bool, GLFW_KEY_LAST + 1> keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> mouse_buttons{false};

	static inline std::array<bool, GLFW_KEY_LAST + 1> last_keys{false};
	static inline std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> last_mouse_buttons{false};
};