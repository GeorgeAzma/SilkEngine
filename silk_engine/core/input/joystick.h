#pragma once

class Joystick
{
public:
	Joystick(int id)
		: id(id) {}

	static void init();

	std::span<const float> getAxes() const;
	std::span<const unsigned char> getButtons() const;
	const char* getGUID() const;
	std::span<const unsigned char> getHats() const;
	const char* getName() const;
	bool isGamepad() const;
	bool isPresent() const;

private:
	int id = -1;
};