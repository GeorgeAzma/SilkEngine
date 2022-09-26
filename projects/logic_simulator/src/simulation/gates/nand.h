#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Nand : public Gate
{
public:
    Nand() : Gate(2, 1) {}

    Gate* clone() const override { return new Nand(*this); }
    operator size_t() const override { return ID<Nand>(); }

    void simulateLogic() override
    {
        unsigned int o = 0;
        for (const auto& i : inputs)
            o += i;
        outputs[0] = o != inputs.size();
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("Nand", as_gui);
    }
};