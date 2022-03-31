#pragma once

class Subrender
{
public:
	virtual void render() = 0;

public:
	bool enabled = true;
};