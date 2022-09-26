#include "input.h"
#include "keys.h"
#include "mouse_buttons.h"
#include "gfx/window/window.h"
#include "gfx/images/raw_image.h"
#include <GLFW/glfw3.h>

void Input::init()
{
	keys.resize(GLFW_KEY_LAST + 1);
	last_keys.resize(GLFW_KEY_LAST + 1);
	mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
	last_mouse_buttons.resize(GLFW_MOUSE_BUTTON_LAST + 1);
	Dispatcher::subscribe(onMousePress);
	Dispatcher::subscribe(onMouseRelease);
	Dispatcher::subscribe(onMouseMove);
	Dispatcher::subscribe(onKeyPress);
	Dispatcher::subscribe(onKeyRelease);
	Dispatcher::subscribe(onJoystickConnect);
}

void Input::update()
{
	memcpy(last_mouse_buttons.data(), mouse_buttons.data(), mouse_buttons.size() * sizeof(bool));
	memcpy(last_keys.data(), keys.data(), keys.size() * sizeof(bool));
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

void Input::setKey(std::string_view name, int key)
{
	key_binds[name] = key;
}

void Input::setMouseButton(std::string_view name, int button)
{
	mouse_button_binds[name] = button;
}

void Input::setJoystickButton(std::string_view name, int button)
{
	joystick_button_binds[name] = button;
}

int Input::getKey(std::string_view name)
{
	return key_binds.at(name);
}

int Input::getMouseButton(std::string_view name)
{
	return mouse_button_binds.at(name);
}

int Input::getJoystickButton(std::string_view name)
{
	return joystick_button_binds.at(name);
}

vec2 Input::getMouse()
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

void Input::setClipboardString(std::string_view str)
{
	glfwSetClipboardString(Window::getGLFWWindow(), str.data());
}

std::string Input::getClipboardString()
{
	return glfwGetClipboardString(Window::getGLFWWindow());
}

InputDevice Input::getActiveInputDevice()
{
	return active_joystick ? InputDevice::JOYSTICK : InputDevice::KEYBOARD_AND_MOUSE;
}

void Input::setCursor(std::string_view file, CursorHotSpot hot_spot)
{
	std::string path = std::string("cursors/") + file.data();
	RawImage<> raw(path);
	GLFWimage cursor_image[1];
	cursor_image[0].height = raw.height;
	cursor_image[0].width = raw.width;
	cursor_image[0].pixels = raw.pixels.data();

	if (cursor)
	{
		glfwDestroyCursor(cursor);
		cursor = nullptr;
	}

	int x, y;
	switch (hot_spot)
	{
	case CursorHotSpot::TOP:
		x = raw.width / 2;
		y = raw.height - 1;
		break;
	case CursorHotSpot::BOTTOM:
		x = raw.width / 2;
		y = 0;
		break;
	case CursorHotSpot::LEFT:
		x = 0;
		y = raw.height / 2;
		break;
	case CursorHotSpot::RIGHT:
		x = raw.width - 1;
		y = raw.height / 2;
		break;
	case CursorHotSpot::TOP_LEFT:
		x = 0;
		y = raw.height - 1;
		break;
	case CursorHotSpot::TOP_RIGHT:
		x = raw.width - 1;
		y = raw.height - 1;
		break;
	case CursorHotSpot::BOTTOM_LEFT:
		x = 0;
		y = 0;
		break;
	case CursorHotSpot::BOTTOM_RIGHT:
		x = raw.width - 1;
		y = 0;
		break;
	case CursorHotSpot::CENTER:
		x = raw.width / 2;
		y = raw.height / 2;
		break;
	default:
		x = raw.width - 1;
		y = raw.height - 1;
		break;
	}

	cursor = glfwCreateCursor(cursor_image, x, y);
	glfwSetCursor(Window::getGLFWWindow(), cursor);
}

const Input::Joystick* Input::getActiveJoystick()
{
	return active_joystick;
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
	mouse = vec2(e.x, e.y);
}

void Input::onKeyPress(const KeyPressEvent& e)
{
	keys[e.key] = true;
}

void Input::onKeyRelease(const KeyReleaseEvent& e)
{
	keys[e.key] = false;
}

void Input::onJoystickConnect(const JoystickEvent& e)
{
	if (e.connected)
	{
		Joystick joy_stick{};
		joy_stick.id = e.id;
		joy_stick.name = glfwGetJoystickName(e.id);
		joysticks.emplace_back(std::move(joy_stick));
		active_joystick = &joysticks.back();
		SK_INFO("Joystick connected");
	}
	else
	{
		if(active_joystick && active_joystick->id == e.id)
			active_joystick = nullptr;
		for (size_t i = 0; i < joysticks.size(); ++i)
		{
			if (joysticks[i].id == e.id)
			{
				joysticks.erase(joysticks.begin() + i);
				break;
			}
		}
		SK_INFO("Joystick disconnected");
	}
}