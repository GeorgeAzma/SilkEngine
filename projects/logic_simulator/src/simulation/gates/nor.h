#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Nor : public Gate
{
public:
    Nor() : Gate(2, 1) {}

    Gate* clone() const override { return new Nor(*this); }
    operator size_t() const override { return ID<Nor>(); }

    void simulateLogic() override
    {
        unsigned int o = 0;
        for (const auto& i : inputs)
            o += i;
        outputs[0] = o == 0;
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("Nor", as_gui);
    }
};