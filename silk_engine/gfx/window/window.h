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

enum class CursorMode
{
    NORMAL,
    HIDDEN,
    LOCKED,
    CAPTURED
};

struct GLFWcursor;
struct GLFWwindow;
struct GLFWmonitor;
class Surface;
class SwapChain;

class Window
{
public:
    Window();
    ~Window();

    void update();
    void recreate();

    uint32_t getWidth() const { return data.width; }
    uint32_t getHeight() const { return data.height; }
    uvec2 getSize() const { return { data.width, data.height }; }
    int32_t getX() const { return data.x; }
    int32_t getY() const { return data.y; }
    float getAspectRatio() const { return (float)data.width / data.height; }
    vec2 getMouse() const { return data.mouse; }
    const Surface& getSurface() const { return *surface; }
    const SwapChain& getSwapChain() const { return *swap_chain; }
    float getOpacity() const { return opacity; }
    CursorMode getCursorMode() const { return cursor_mode; }
    GLFWmonitor* getMonitor() const;

    bool isFullscreen() const { return data.fullscreen; }
    bool isTransparent() const { return transparent; }
    bool isMinimized() const { return data.minimized || data.width == 0 || data.height == 0; }
    bool isMaximized() const { return data.maximized; }
    bool isFocused() const { return data.focused; }
    
    bool isMouseHeld(int button) const { return data.mouse_buttons[button]; }
    bool isMousePressed(int button) const { return data.isMousePressed(button); }
    bool isMouseReleased(int button) const { return data.isMouseReleased(button); }
    bool isMouseOn() const { return data.is_mouse_on; }
    
    bool isKeyHeld(int key) const { return data.keys[key]; }
    bool isKeyPressed(int key) const { return data.isKeyPressed(key); }
    bool isKeyReleased(int key) const { return data.isKeyReleased(key); }
    
    bool isResizable() const { return resizable; }
    bool isDecorated() const { return decorated; }
    bool isAlwaysOnTop() const { return always_on_top; }
    bool isAutoMinify() const { return auto_minify; }
    bool isVsync() const { return vsync; }
    bool shouldClose() const;
   
    void setFullscreen(bool fullscreen);
    void toggleFullscreen() { setFullscreen(!isFullscreen()); }
    void setX(int32_t x);
    void setY(int32_t y);
    void setPosition(const ivec2 &position);
    void setWidth(uint32_t width);
    void setHeight(uint32_t height);
    void setSize(const uvec2 &size);
    void setTitle(std::string_view title);
    void setIcon(const path& file);
    void setSizeLimit(uint32_t min_width, uint32_t min_height, uint32_t max_width, uint32_t max_height);
    void setAspectRatioLimit(uint32_t numerator, uint32_t denominator);
	void setCursor(const path& file, CursorHotSpot hot_spot = CursorHotSpot::TOP_RIGHT);
    void setOpacity(float opacity);
    void setResizable(bool resizable);
    void setDecorated(bool decorated);
    void setAlwaysOnTop(bool always_on_top);
    void setAutoMinify(bool auto_minify);
    void setFocusOnShow(bool focus_on_show);
	void setCursorMode(CursorMode mode);
    void setVsync(bool vsync);

    void align(WindowAlignment a = WindowAlignment::CENTER);
    void focus();
    void show();
    void hide();
    void minimize();
    void maximize();
    void restore();
    void requestAttention();

    operator GLFWwindow* () const { return window; }

private:
    GLFWwindow *window = nullptr;

    struct UserPointer
    {
        Window* window = nullptr;
        int x = 0;
        int y = 0;
        uint32_t width = 1280;
        uint32_t height = 720;
        std::string title = "Window";
        bool fullscreen = false;
        int mouse_pressed = -1;
        bool is_mouse_on = false;
        std::vector<uint8_t> keys;
        std::vector<uint8_t> last_keys;
        std::vector<uint8_t> mouse_buttons;
        std::vector<uint8_t> last_mouse_buttons;
        vec2 mouse = vec2(0);
        GLFWcursor* cursor = nullptr;
        bool minimized = false;
        bool maximized = false;
        bool focused = false;
        vec2 dpi = vec2(0);

        bool isMousePressed(int button) const { return mouse_buttons[button] && !last_mouse_buttons[button]; }
        bool isMouseReleased(int button) const { return !mouse_buttons[button] && last_mouse_buttons[button]; }
        bool isKeyPressed(int key) const { return keys[key] && !last_keys[key]; }
        bool isKeyReleased(int key) const { return !keys[key] && last_keys[key]; }
    } data;

	struct WindowedData
	{
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	} windowed;

    float opacity = 1.0f;
    float resizable = true;
    bool decorated = true;
    bool always_on_top = false;
    bool transparent = false;
    bool auto_minify = false;
    bool focus_on_show = false;
    bool vsync = false;
    CursorMode cursor_mode = CursorMode::NORMAL;

    Surface* surface = nullptr;
    SwapChain* swap_chain = nullptr;

public:
    static Window& getActive() { return *active_window; }
    static void setActive(Window* window) { active_window = window; }

private:
    static inline Window* active_window = nullptr;
};