#pragma once

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
	CYAN
};

class Color
{
	enum class Type 
	{
		RGBA, ARGB, RGB, 
		BGRA, ABGR, BGR
	};
	static constexpr float NORM = 1.0f / 255.0f;

public:
	Color(float r, float g, float b, float a = 1.0f);
	Color(const glm::vec4& color) : Color(color.r, color.g, color.b, color.a) {}
	Color(uint32_t hex, Type type = Type::RGBA);
	Color(std::string hex, float a = 1.0f);
	Color(Colors color);

	constexpr uint32_t getHex(Type type = Type::RGBA) const;
	std::string getHexString() const;

	operator glm::vec4&() { return color; }
	operator const glm::vec4&() const { return color; }
	operator glm::u8vec4() const { return color * 255.0f; }
	Color& operator=(const glm::vec4& color) { this->color = color; return *this; }
	Color& operator=(const glm::vec3& color) { this->color = { color.r, color.g, color.b, 1.0f }; return *this; }
	Color& operator=(Colors color) { this->color = Color(color); return *this; }

private:
	glm::vec4 color;
};