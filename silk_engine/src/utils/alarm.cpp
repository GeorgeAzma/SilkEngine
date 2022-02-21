#include "alarm.h"

Alarm::Alarm(const Time& time, bool repeat)
	: start_time(Time::now()), now(start_time), time(time.asMicroseconds()), repeat(repeat)
{
}

Alarm::operator bool()
{
	if(repeat) 
		up = false;

	now = Time::now();
	uint64_t delta = (now - start_time).asMicroseconds();

	if (delta >= time)
	{
		start_time = now;
		up = true;
	}

	return up;
}
