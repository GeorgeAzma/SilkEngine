#pragma once

class Subrender
{
public:
	Subrender() = default;
	virtual ~Subrender() = default;

	virtual void render() = 0;

public:
	bool enabled = true;
};