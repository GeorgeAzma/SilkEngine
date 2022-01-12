#include "input.h"
#include "keys.h"
#include "mouse_buttons.h"

void Input::init()
{
	Dispatcher::subscribe(onMousePress);
	Dispatcher::subscribe(onMouseRelease);
	Dispatcher::subscribe(onKeyPress);
	Dispatcher::subscribe(onKeyRelease);
}

inline bool Input::isMouseDown(int button)
{
	return mouse_buttons[button];
}

inline bool Input::isKeyDown(int key)
{
	return keys[key];
}

inline bool Input::isMousePressed(int button)
{
	return mouse_buttons[button] && !last_mouse_buttons[button];
}

inline bool Input::isKeyPressed(int key)
{
	return keys[key] && !last_keys[key];
}

inline bool Input::isMouseReleased(int button)
{
	return !mouse_buttons[button] && last_mouse_buttons[button];
}

inline bool Input::isKeyReleased(int key)
{
	return !keys[key] && last_keys[key];
}

void Input::onMousePress(const MousePressEvent& e)
{
	mouse_buttons[e.button] = true;
}

void Input::onMouseRelease(const MouseReleaseEvent& e)
{
	mouse_buttons[e.button] = false;
}

void Input::onKeyPress(const KeyPressEvent& e)
{
	keys[e.key] = true;
}

void Input::onKeyRelease(const KeyReleaseEvent& e)
{
	keys[e.key] = false;
}
