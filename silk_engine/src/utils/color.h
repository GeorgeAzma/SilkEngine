#pragma once

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
	Color(uint32_t hex, Type type = Type::RGBA);
	Color(std::string hex, float a = 1.0f);

	constexpr uint32_t getHex(Type type = Type::RGBA) const;
	std::string getHexString() const;

	operator glm::vec4&() { return color; }
	operator const glm::vec4&() const { return color; }

private:
	glm::vec4 color;
};