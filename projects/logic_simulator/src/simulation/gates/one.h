#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class One : public Gate
{
public:
    One() : Gate(0, 1) {}

    Gate* clone() const override { return new One(*this); }
    operator size_t() const override { return ID<One>(); }

    void simulateLogic() override
    {
        outputs[0] = true;
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("1", as_gui, constant_type_color);
    }
};