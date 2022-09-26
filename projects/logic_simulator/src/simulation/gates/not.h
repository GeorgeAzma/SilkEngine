#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Not : public Gate
{
public:
    Not() : Gate(1, 1) {}

    Gate* clone() const override { return new Not(*this); }
    operator size_t() const override { return ID<Not>(); }

    void simulateLogic() override
    {
        outputs[0] = !inputs[0];
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("Not", as_gui);
    }
};