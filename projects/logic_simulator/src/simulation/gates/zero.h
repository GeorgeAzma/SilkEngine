#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Zero : public Gate
{
public:
    Zero() : Gate(0, 1) {}

    Gate* clone() const override { return new Zero(*this); }
    operator size_t() const override { return ID<Zero>(); }

    void simulateLogic() override
    {
        outputs[0] = false;
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("0", as_gui, constant_type_color);
    }
};