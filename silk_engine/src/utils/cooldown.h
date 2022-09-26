#pragma once

class Cooldown
{
public:
	Cooldown(float cooldown = 0.0f)
		: cooldown(cooldown), time(0.0f) {}
	template<typename T, typename P>
	Cooldown(const std::chrono::duration<T, P>& duration)
		: Cooldown(std::chrono::duration_cast<std::chrono::duration<float>>(duration).count()) {}

	bool operator()()
	{
		if (time >= cooldown)
		{
			time = Time::dt;
			return true;
		}
		time += Time::dt;
		return false;
	}

	operator bool() { return time >= cooldown; }

public:
	float cooldown = 0.0f;

private:
	float time = 0.0f;
};
