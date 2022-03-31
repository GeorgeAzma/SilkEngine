#include "color.h"

Color::Color(float r, float g, float b, float a)
	: color(r, g, b, a)
{
}

Color::Color(uint32_t hex, Type type)
{
	switch (type) 
	{
	case Type::RGBA:
		color.r = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.b = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.a = float(uint8_t(hex & 0xFF)) * NORM;
		break;
	case Type::ARGB:
		color.a = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.r = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.b = float(uint8_t(hex & 0xFF)) * NORM;
		break;
	case Type::RGB:
		color.r = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.b = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.a = 1.0f;
		break;

	case Type::BGRA:
		color.b = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.r = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.a = float(uint8_t(hex & 0xFF)) * NORM;
		break;
	case Type::ABGR:
		color.a = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.b = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.r = float(uint8_t(hex & 0xFF)) * NORM;
		break;
	case Type::BGR:
		color.b = float(uint8_t(hex >> 24 & 0xFF)) * NORM;
		color.g = float(uint8_t(hex >> 16 & 0xFF)) * NORM;
		color.r = float(uint8_t(hex >> 8 & 0xFF)) * NORM;
		color.a = 1.0f;
		break;
	default:
		SK_ERROR("Unknown Color type");
	}
}

Color::Color(std::string hex, float a)
{
	color.a = a;
	if (hex[0] == '#')
		hex.erase(0, 1);

	SK_ASSERT(hex.size() == 6, "Invalid hex string size");
	auto hex_value = std::stoul(hex, nullptr, 16);

	color.r = float((hex_value >> 16) & 0xff) * NORM;
	color.g = float((hex_value >> 8) & 0xff) * NORM;
	color.b = float((hex_value >> 0) & 0xff) * NORM;
}

constexpr uint32_t Color::getHex(Type type) const
{
	switch (type) {
	case Type::RGBA:
		return (uint8_t(color.r * 255.0f) << 24) | (uint8_t(color.g * 255.0f) << 16) | (uint8_t(color.b * 255.0f) << 8) | (uint8_t(color.a * 255.0f) & 0xFF);
	case Type::ARGB:
		return (uint8_t(color.a * 255.0f) << 24) | (uint8_t(color.r * 255.0f) << 16) | (uint8_t(color.g * 255.0f) << 8) | (uint8_t(color.b * 255.0f) & 0xFF);
	case Type::RGB:
		return (uint8_t(color.r * 255.0f) << 16) | (uint8_t(color.g * 255.0f) << 8) | (uint8_t(color.b * 255.0f) & 0xFF);
	default:
		SK_ERROR("Unknown Color type");
	}
}

std::string Color::getHexString() const
{
	std::stringstream stream;
	stream << "#";

	auto hexValue = ((uint32_t(color.r * 255.0f) & 0xFF) << 16) +
					((uint32_t(color.g * 255.0f) & 0xFF) << 8) +
					(uint32_t(color.b * 255.0f) & 0xFF);
	stream << std::hex << std::setfill('0') << std::setw(6) << hexValue;

	return stream.str();
}
