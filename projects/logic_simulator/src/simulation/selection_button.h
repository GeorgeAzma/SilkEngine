#pragma once

#include "core/event.h"

class SelectionButton
{
public:
	SelectionButton() {}
	SelectionButton(float x, float y, float width, float height);

	bool onMousePress(const MousePressEvent& e);

	void render() const;

	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
};