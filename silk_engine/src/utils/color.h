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

class Color : public glm::vec4
{
	enum class Type
	{
		RGBA, ARGB, RGB,
		BGRA, ABGR, BGR
	};
	static constexpr float N = 1.0f / 255.0f;

public:
	Color(float r, float g, float b, float a = 1.0f);
	Color(float r, float g) : Color(r, g, 0.0f, 1.0f) {}
	Color(float grayscale = 0.0f) : Color(grayscale, grayscale, grayscale, 1.0f) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : Color(r* N, g* N, b* N, a* N) {}
	Color(uint8_t r, uint8_t g) : Color(r * N, g * N, 1.0f, 1.0f) {}
	Color(uint8_t grayscale = 0) : Color(grayscale * N, grayscale * N, grayscale * N, 1.0f) {}
	Color(const glm::vec4& color) : Color(color.r, color.g, color.b, color.a) {}
	Color(const glm::vec3& color) : Color(color.r, color.g, color.b, 1.0f) {}
	Color(const glm::u8vec4& color) : Color(glm::vec4(color) * N) {}
	Color(const glm::u8vec3& color) : Color(glm::vec4(color, 255) * N) {}
	Color(uint32_t hex, Type type = Type::RGBA);
	Color(std::string hex, float a = 1.0f);
	Color(Colors color);

	constexpr uint32_t getHex(Type type = Type::RGBA) const;
	std::string getHexString() const;

	operator glm::vec3() const { return glm::vec3(r, g, b); }
	operator glm::u8vec3() const { return glm::vec3(r, g, b) * 255.0f; }
	operator glm::u8vec4() const { return *this * 255.0f; }
};