#pragma once

class System : NonCopyable
{
public:
	virtual ~System() = default;

	virtual void update() = 0;

public:
	bool enabled = true;
};