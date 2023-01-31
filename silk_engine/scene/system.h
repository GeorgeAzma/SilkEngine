#pragma once

class System : NonCopyable
{
public:
	virtual ~System() = default;

	virtual void init() {}
	virtual void update() {}
	virtual void destroy() {}

public:
	bool enabled = true;
};