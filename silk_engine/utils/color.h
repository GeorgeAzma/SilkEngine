#pragma once
#undef TRANSPARENT

enum class Colors
{
	WHITE,
	BLACK,
	DARK_GRAY,
	GRAY,
	LIGHT_GRAY,
	RED,
	GREEN,
	BLUE,
	PINK,
	MAGENTA,
	PURPLE,
	YELLOW,
	ORANGE,
	BROWN,
	CYAN,
	TRANSPARENT
};

class Color : public vec4
{
	enum class Type
	{
		RGBA, ARGB, RGB,
		BGRA, ABGR, BGR
	};
	static constexpr float N = 1.0f / 255.0f;

public:
	constexpr Color(float r, float g, float b, float a = 1.0f) : vec4(r, g, b, a) {}
	constexpr Color(float r, float g) : Color(r, g, 0.0f, 1.0f) {}
	constexpr Color(float grayscale = 0.0f) : Color(grayscale, grayscale, grayscale, 1.0f) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : Color(r* N, g* N, b* N, a* N) {}
	constexpr Color(uint8_t r, uint8_t g) : Color(r * N, g * N, 1.0f, 1.0f) {}
	constexpr Color(uint8_t grayscale = 0) : Color(grayscale * N, grayscale * N, grayscale * N, 1.0f) {}
	constexpr Color(const vec4& color) : Color(color.r, color.g, color.b, color.a) {}
	constexpr Color(const vec3& color) : Color(color.r, color.g, color.b, 1.0f) {}
	constexpr Color(const u8vec4& color) : Color(vec4(color) * N) {}
	constexpr Color(const u8vec3& color) : Color(vec4(color, 255) * N) {}
	constexpr Color(uint32_t hex, Type type = Type::RGBA);
	Color(std::string hex, float a = 1.0f);
	constexpr Color(Colors color)
	{
		switch (color)
		{
		case Colors::WHITE:
			*this = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;
		case Colors::BLACK:
			*this = { 0.0f, 0.0f, 0.0f, 1.0f };
			break;
		case Colors::DARK_GRAY:
			*this = { 0.25f, 0.25f, 0.25f, 1.0f };
			break;
		case Colors::GRAY:
			*this = { 0.5f, 0.5f, 0.5f, 1.0f };
			break;
		case Colors::LIGHT_GRAY:
			*this = { 0.75f, 0.75f, 0.75f, 1.0f };
			break;
		case Colors::RED:
			*this = { 1.0f, 0.0f, 0.0f, 1.0f };
			break;
		case Colors::GREEN:
			*this = { 0.0f, 1.0f, 0.0f, 1.0f };
			break;
		case Colors::BLUE:
			*this = { 0.0f, 0.0f, 1.0f, 1.0f };
			break;
		case Colors::PINK:
			*this = { 1.0f, 0.5f, 0.75f, 1.0f };
			break;
		case Colors::MAGENTA:
			*this = { 1.0f, 0.0f, 1.0f, 1.0f };
			break;
		case Colors::PURPLE:
			*this = { 0.5f, 0.0f, 0.5f, 1.0f };
			break;
		case Colors::YELLOW:
			*this = { 1.0f, 1.0f, 0.0f, 1.0f };
			break;
		case Colors::ORANGE:
			*this = { 1.0f, 0.5f, 0.0f, 1.0f };
			break;
		case Colors::BROWN:
			*this = { 0.5f, 0.25f, 0.0f, 1.0f };
			break;
		case Colors::CYAN:
			*this = { 0.0f, 1.0f, 1.0f, 1.0f };
			break;
		case Colors::TRANSPARENT:
			*this = { 0.0f, 0.0f, 0.0f, 0.0f };
			break;
		default:
			*this = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;
		}
	}

	constexpr uint32_t getHex(Type type = Type::RGBA) const;
	std::string getHexString() const;

	constexpr operator vec3() const { return vec3(r, g, b); }
	constexpr operator u8vec3() const { return vec3(r, g, b) * 255.0f; }
	constexpr operator u8vec4() const { return *this * 255.0f; }
};