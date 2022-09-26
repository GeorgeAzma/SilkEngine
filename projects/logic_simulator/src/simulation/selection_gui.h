#pragma once

#include "core/event.h"
#include "gates/gate.h"
#include "selection_button.h"

class SelectionGui
{
public:
	SelectionGui();

	Gate* onMousePress(const MousePressEvent& e);

	void render();

	std::vector<SelectionButton> buttons{};
	std::vector<shared<Gate>> gates{};
};