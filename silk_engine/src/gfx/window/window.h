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

struct GLFWwindow;

enum class WindowAlignment
{
    NONE,
    CENTER,
    LEFT,
    RIGHT,
    BOTTOM,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP,
    TOP_LEFT,
    TOP_RIGHT,
};

class Window
{
public:
    static void init();
	static void update();
    static void destroy();

    static uint32_t getWidth() { return data.width; }
    static uint32_t getHeight() { return data.height; }
    static uvec2 getSize() { return { data.width, data.height }; }
    static int32_t getX() { return data.x; }
    static int32_t getY() { return data.y; }
    static float getAspectRatio() { return (float)data.width / data.height; }
    static GLFWwindow *getGLFWWindow() { return window; }
    static vec2 getMouse() { return input.mouse; }

    static bool isFullscreen() { return data.fullscreen; }
    static bool isVsync() { return vsync; }
    static bool isTransparent() { return transparent; }
    static bool isMinimized() { return data.width == 0 || data.height == 0; }
	static bool isMouseDown(int button) { return input.mouse_buttons[button]; }
	static bool isKeyDown(int key) { return input.keys[key]; }
    static bool isMousePressed(int button) { return input.isMousePressed(button); }
	static bool isKeyPressed(int key) { return input.isKeyPressed(key); }
	static bool isMouseReleased(int button) { return input.isMouseReleased(button); }
	static bool isKeyReleased(int key) { return input.isKeyReleased(key); }
    static bool isMouseOn() { return input.is_mouse_on; }

	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onMouseMove(const MouseMoveEvent& e);
	static void onKeyPress(const KeyPressEvent& e);
	static void onKeyRelease(const KeyReleaseEvent& e);
	static void onMouseEnter(const MouseEnterEvent& e);
	static void onMouseLeave(const MouseLeaveEvent& e);
   
    static void setVsync(bool vsync);
    static void setFullscreen(bool fullscreen);
    static void setX(int32_t x);
    static void setY(int32_t y);
    static void setPosition(const ivec2 &position);
    static void setWidth(uint32_t width);
    static void setHeight(uint32_t height);
    static void setSize(const uvec2 &size);
    static void setTitle(std::string_view title);
    static void setIcon(std::string_view file);
    static void setSizeLimits(uint32_t min_width, uint32_t max_width, uint32_t min_height, uint32_t max_height);
    static void setAspectRatio(uint32_t numerator, uint32_t denominator);
	static void setCursor(std::string_view file, CursorHotSpot hot_spot = CursorHotSpot::TOP_RIGHT);

    static void align(WindowAlignment a = WindowAlignment::CENTER);
    static void focus();
    static void show();
    static void hide();
	static void lockMouse();
	static void unlockMouse();

private:
    static inline GLFWwindow *window = nullptr;

    static inline struct UserPointer
    {
        uint32_t width = 1280;
        uint32_t height = 720;
        std::string title = "Window";
        int x = 0;
        int y = 0;
        bool fullscreen = false;
        int mouse_pressed = -1;
    } data;

	static inline struct WindowedData
	{
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	} windowed;

	static inline struct InputData
	{
		std::vector<uint8_t> keys;
		std::vector<uint8_t> last_keys;
		std::vector<uint8_t> mouse_buttons;
		std::vector<uint8_t> last_mouse_buttons;
		vec2 mouse = vec2(0);
		bool is_mouse_on = false;
		GLFWcursor* cursor = nullptr;

        bool isMousePressed(int button) const { return mouse_buttons[button] && !last_mouse_buttons[button]; }
        bool isKeyPressed(int key) const { return keys[key] && !last_keys[key]; }
        bool isMouseReleased(int button) const { return !mouse_buttons[button] && last_mouse_buttons[button]; }
        bool isKeyReleased(int key) const { return !keys[key] && last_keys[key]; }
	} input;

    static inline bool vsync = false;
    static inline bool transparent = false;
    static inline bool decorated = true;
};