#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Xor : public Gate
{
public:
    Xor() : Gate(2, 1) {}

    Gate* clone() const override { return new Xor(*this); }
    operator size_t() const override { return ID<Xor>(); }

    void simulateLogic() override
    {
        unsigned int o = 0;
        for (const auto& i : inputs)
            o += i;
        outputs[0] = o > 0 && o != inputs.size();
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("Xor", as_gui);
    }
};