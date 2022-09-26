#include "color.h"

constexpr Color::Color(uint32_t hex, Type type)
{
	switch (type) 
	{
	case Type::RGBA:
		r = float(uint8_t(hex >> 24 & 0xFF)) * N;
		g = float(uint8_t(hex >> 16 & 0xFF)) * N;
		b = float(uint8_t(hex >> 8 & 0xFF)) * N;
		a = float(uint8_t(hex & 0xFF)) * N;
		break;
	case Type::ARGB:
		a = float(uint8_t(hex >> 24 & 0xFF)) * N;
		r = float(uint8_t(hex >> 16 & 0xFF)) * N;
		g = float(uint8_t(hex >> 8 & 0xFF)) * N;
		b = float(uint8_t(hex & 0xFF)) * N;
		break;
	case Type::RGB:
		r = float(uint8_t(hex >> 24 & 0xFF)) * N;
		g = float(uint8_t(hex >> 16 & 0xFF)) * N;
		b = float(uint8_t(hex >> 8 & 0xFF)) * N;
		a = 1.0f;
		break;
	case Type::BGRA:
		b = float(uint8_t(hex >> 24 & 0xFF)) * N;
		g = float(uint8_t(hex >> 16 & 0xFF)) * N;
		r = float(uint8_t(hex >> 8 & 0xFF)) * N;
		a = float(uint8_t(hex & 0xFF)) * N;
		break;
	case Type::ABGR:
		a = float(uint8_t(hex >> 24 & 0xFF)) * N;
		b = float(uint8_t(hex >> 16 & 0xFF)) * N;
		g = float(uint8_t(hex >> 8 & 0xFF)) * N;
		r = float(uint8_t(hex & 0xFF)) * N;
		break;
	case Type::BGR:
		b = float(uint8_t(hex >> 24 & 0xFF)) * N;
		g = float(uint8_t(hex >> 16 & 0xFF)) * N;
		r = float(uint8_t(hex >> 8 & 0xFF)) * N;
		a = 1.0f;
		break;
	default:
		SK_ERROR("Unknown Color type");
	}
}

Color::Color(std::string hex, float a)
{
	this->a = a;
	if (hex[0] == '#')
		hex.erase(0, 1);

	SK_ASSERT(hex.size() == 6, "Invalid hex string size");
	auto hex_value = std::stoul(hex, nullptr, 16);

	r = float((hex_value >> 16) & 0xff) * N;
	g = float((hex_value >> 8) & 0xff) * N;
	b = float((hex_value >> 0) & 0xff) * N;
}

constexpr uint32_t Color::getHex(Type type) const
{
	switch (type) {
	case Type::RGBA:
		return (uint8_t(r * 255.0f) << 24) | (uint8_t(g * 255.0f) << 16) | (uint8_t(b * 255.0f) << 8) | (uint8_t(a * 255.0f) & 0xFF);
	case Type::ARGB:
		return (uint8_t(a * 255.0f) << 24) | (uint8_t(r * 255.0f) << 16) | (uint8_t(g * 255.0f) << 8) | (uint8_t(b * 255.0f) & 0xFF);
	case Type::RGB:
		return (uint8_t(r * 255.0f) << 16) | (uint8_t(g * 255.0f) << 8) | (uint8_t(b * 255.0f) & 0xFF);
	default:
		SK_ERROR("Unknown Color type");
	}
}

std::string Color::getHexString() const
{
	std::stringstream stream;
	stream << "#";

	auto hexValue = ((uint32_t(r * 255.0f) & 0xFF) << 16) +
					((uint32_t(g * 255.0f) & 0xFF) << 8) +
					(uint32_t(b * 255.0f) & 0xFF);
	stream << std::hex << std::setfill('0') << std::setw(6) << hexValue;

	return stream.str();
}
