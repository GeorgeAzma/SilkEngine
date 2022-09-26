#pragma once

#include "gate.h"
#include "gfx/renderer.h"
#include "core/input/input.h"
#include "core/input/mouse_buttons.h"

class Bulb : public Gate
{
public:
    Bulb() : Gate(1, 0) {}

    Gate* clone() const override { return new Bulb(*this); }
    operator size_t() const override { return ID<Bulb>(); }

    void render(bool as_gui = false) const override
    {
        renderGeneric("", as_gui, inputs[0] ? Color(1.0f, 0.88f, 0.85f) : Color(0.1f));
    }
};