#pragma once

enum class Key : int
{
	SPACE = 32,
	APOSTROPHE = 39,
	COMMA = 44,
	MINUS = 45,
	PERIOD = 46,
	SLASH = 47,
	_0 = 48,
	_1 = 49,
	_2 = 50,
	_3 = 51,
	_4 = 52,
	_5 = 53,
	_6 = 54,
	_7 = 55,
	_8 = 56,
	_9 = 57,
	SEMICOLON = 59,
	EQUAL = 61,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	LEFT_BRACKET = 91,  // [
	BACKSLASH = 92,  // \ 
	RIGHT_BRACKET = 93,  // ]
	GRAVE_ACCENT = 96,  // `
	WORLD_1 = 161, // non-US #1
	WORLD_2 = 162, // non-US #2

	ESCAPE = 256,
	ENTER = 257,
	TAB = 258,
	BACKSPACE = 259,
	INSERT = 260,
	DEL = 261,
	RIGHT = 262,
	LEFT = 263,
	DOWN = 264,
	UP = 265,
	PAGE_UP = 266,
	PAGE_DOWN = 267,
	HOME = 268,
	END = 269,
	CAPS_LOCK = 280,
	SCROLL_LOCK = 281,
	NUM_LOCK = 282,
	PRINT_SCREEN = 283,
	PAUSE = 284,
	F1 = 290,
	F2 = 291,
	F3 = 292,
	F4 = 293,
	F5 = 294,
	F6 = 295,
	F7 = 296,
	F8 = 297,
	F9 = 298,
	F10 = 299,
	F11 = 300,
	F12 = 301,
	F13 = 302,
	F14 = 303,
	F15 = 304,
	F16 = 305,
	F17 = 306,
	F18 = 307,
	F19 = 308,
	F20 = 309,
	F21 = 310,
	F22 = 311,
	F23 = 312,
	F24 = 313,
	F25 = 314,
	KP_0 = 320,
	KP_1 = 321,
	KP_2 = 322,
	KP_3 = 323,
	KP_4 = 324,
	KP_5 = 325,
	KP_6 = 326,
	KP_7 = 327,
	KP_8 = 328,
	KP_9 = 329,
	KP_DECIMAL = 330,
	KP_DIVIDE = 331,
	KP_MULTIPLY = 332,
	KP_SUBTRACT = 333,
	KP_ADD = 334,
	KP_ENTER = 335,
	KP_EQUAL = 336,
	LEFT_SHIFT = 340,
	LEFT_CONTROL = 341,
	LEFT_ALT = 342,
	LEFT_SUPER = 343,
	RIGHT_SHIFT = 344,
	RIGHT_CONTROL = 345,
	RIGHT_ALT = 346,
	RIGHT_SUPER = 347,
	MENU = 348,
	LAST = MENU
};

enum class MouseButton : int
{
	UNKNOWN = -1,
	_1 = 0,
	_2 = 1,
	_3 = 2,
	_4 = 3,
	_5 = 4,
	_6 = 5,
	_7 = 6,
	_8 = 7,
	LAST = _8,

	LEFT = _1,
	RIGHT = _2,
	MIDDLE = _3
};

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
	uint32_t getFramebufferWidth() const { return data.framebuffer_width; }
	uint32_t getFramebufferHeight() const { return data.framebuffer_height; }
    uvec2 getSize() const { return { data.width, data.height }; }
    int32_t getX() const { return data.x; }
    int32_t getY() const { return data.y; }
    float getAspectRatio() const { return (float)data.width / data.height; }
	float getContentScaleX() const { return data.content_scale_x; }
	float getContentScaleY() const { return data.content_scale_y; }
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
    
    bool isKeyHeld(Key key) const { return data.keys[ecast(key)]; }
    bool isKeyPressed(Key key) const { return data.isKeyPressed(key); }
    bool isKeyReleased(Key key) const { return data.isKeyReleased(key); }

	bool isMouseHeld(MouseButton button) const { return data.mouse_buttons[ecast(button)]; }
	bool isMousePressed(MouseButton button) const { return data.isMousePressed(button); }
	bool isMouseReleased(MouseButton button) const { return data.isMouseReleased(button); }
	bool isMouseOn() const { return data.is_mouse_on; }
    
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
    void setIcon(const fs::path& file);
    void setSizeLimit(uint32_t min_width, uint32_t min_height, uint32_t max_width, uint32_t max_height);
    void setAspectRatioLimit(uint32_t numerator, uint32_t denominator);
	void setCursor(const fs::path& file, CursorHotSpot hot_spot = CursorHotSpot::TOP_RIGHT);
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
	void close();

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
		uint32_t framebuffer_width = 1280;
		uint32_t framebuffer_height = 720;
		float content_scale_x = 1.0f;
		float content_scale_y = 1.0f;
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

        bool isKeyPressed(Key key) const { return keys[ecast(key)] && !last_keys[ecast(key)]; }
        bool isKeyReleased(Key key) const { return !keys[ecast(key)] && last_keys[ecast(key)]; }
        bool isMousePressed(MouseButton button) const { return mouse_buttons[ecast(button)] && !last_mouse_buttons[ecast(button)]; }
        bool isMouseReleased(MouseButton button) const { return !mouse_buttons[ecast(button)] && last_mouse_buttons[ecast(button)]; }
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
	static const std::unordered_set<Window*>& getWindows() { return windows; }
	static void setActive(Window* window);

private:
    static inline Window* active_window = nullptr;
	static inline std::unordered_set<Window*> windows = {};
};