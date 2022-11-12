#pragma once

#include "gate.h"
#include "gfx/renderer.h"
#include "gfx/window/window.h"
#include "core/input/mouse_buttons.h"

class Button : public Gate
{
public:
    Button() : Gate(0, 1) {}

    Gate* clone() const override { return new Button(*this); }
    operator size_t() const override { return ID<Button>(); }

    void onMousePress(const MousePressEvent& e)
    {
        outputs[0] = true;
        updateOutputs();
    }

    void onMouseRelease(const MouseReleaseEvent& e)
    {
        outputs[0] = false;
        updateOutputs();
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("", as_gui, vec4{ (vec3(misc_type_color) + vec3(0.1f, -0.1f, 0.2f)) * (1.0f - outputs[0] * 0.4f), misc_type_color.a }, true);
    }
};