#pragma once

#include "time.h"

class Alarm
{
public:
	Alarm(const Time& time, bool repeat = true);

	operator bool();

	const Time& lastSetoff() const { return start_time; }

private:
	Time start_time;
	Time now;
	uint64_t time;
	bool repeat;
	bool up = false;
};