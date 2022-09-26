#pragma once

#include "gates/gate.h"
#include "core/event.h"
#include "selection_gui.h"

class Simulation
{
	struct Pin
	{
		size_t index = 0;
		size_t layer = 0;
		shared<Gate> gate = nullptr;
		operator bool() const { return gate.get(); }
	};
public:
	static void init();
	static void update();
	static void onMousePress(const MousePressEvent& e);
	static void onMouseRelease(const MouseReleaseEvent& e);
	static void onMouseScroll(const MouseScrollEvent& e);
	static void destroy();

	static void save();
	static void load();

private:
	static inline std::vector<shared<Gate>> gates{};
	static inline SelectionGui selection_gui{};
	static inline shared<Gate> moving_gate = nullptr;
	static inline vec2 start_pos = vec2(0);
	static inline vec2 offset = vec2(0);
	static inline Pin selected_pin{};
};