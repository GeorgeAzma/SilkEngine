#pragma once

class Joystick
{
public:
	Joystick(int id)
		: id(id) {}

	static void init();

	const char* getName();

	static Joystick getActive();

	bool operator==(const Joystick& other) const { return id == other.id; }
	bool operator==(int id) const { return this->id == id; }
	bool isValid() const { return id != -1; }

private:
	int id = -1;

	static inline std::vector<Joystick> joysticks;
	static Joystick active_joystick;
};