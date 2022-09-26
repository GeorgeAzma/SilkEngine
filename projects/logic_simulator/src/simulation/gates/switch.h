#pragma once

#include "gate.h"
#include "gfx/renderer.h"
#include "core/input/input.h"
#include "core/input/mouse_buttons.h"

class Switch : public Gate
{
public:
    Switch() : Gate(0, 1) {}

    Gate* clone() const override { return new Switch(*this); }
    operator size_t() const override { return ID<Switch>(); }

    void simulateLogic() override
    {
        if (math::isPointInRectangle(Input::getMouse(), { x, y, getWidth(), getHeight() }))
            outputs[0] = !outputs[0];
    }

    void onMouseRelease(const MouseReleaseEvent& e) override
    {
        if (!moved && e.button == MouseButtons::LEFT)
            updateOutputs();
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("", as_gui, vec4{ vec3(misc_type_color) * (1.0f - outputs[0] * 0.4f), misc_type_color.a });
    }
};