#include "input.h"
#include "keys.h"
#include "mouse_buttons.h"
#include "core/window.h"

void Input::init()
{
	Dispatcher::subscribe(onMousePress);
	Dispatcher::subscribe(onMouseRelease);
	Dispatcher::subscribe(onMouseMove);
	Dispatcher::subscribe(onKeyPress);
	Dispatcher::subscribe(onKeyRelease);
}

bool Input::isMouseDown(int button)
{
	return mouse_buttons[button];
}

bool Input::isKeyDown(int key)
{
	return keys[key];
}

bool Input::isMousePressed(int button)
{
	return mouse_buttons[button] && !last_mouse_buttons[button];
}

bool Input::isKeyPressed(int key)
{
	return keys[key] && !last_keys[key];
}

bool Input::isMouseReleased(int button)
{
	return !mouse_buttons[button] && last_mouse_buttons[button];
}

bool Input::isKeyReleased(int key)
{
	return !keys[key] && last_keys[key];
}

glm::vec2 Input::getMouse()
{
	return mouse;
}

float Input::getMouseX()
{
	return mouse.x;
}

float Input::getMouseY()
{
	return mouse.y;
}

void Input::lockMouse()
{
	glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::unlockMouse()
{
	glfwSetInputMode(Window::getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Input::onMousePress(const MousePressEvent& e)
{
	mouse_buttons[e.button] = true;
}

void Input::onMouseRelease(const MouseReleaseEvent& e)
{
	mouse_buttons[e.button] = false;
}

void Input::onMouseMove(const MouseMoveEvent& e)
{
	mouse = glm::vec2(e.x, e.y);
}

void Input::onKeyPress(const KeyPressEvent& e)
{
	keys[e.key] = true;
}

void Input::onKeyRelease(const KeyReleaseEvent& e)
{
	keys[e.key] = false;
}
