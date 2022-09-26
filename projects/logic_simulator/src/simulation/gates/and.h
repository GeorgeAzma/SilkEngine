#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class And : public Gate
{
public:
    And() : Gate(2, 1) {}

    Gate* clone() const override { return new And(*this); }
    operator size_t() const override { return ID<And>(); }

    void simulateLogic() override
    {
        unsigned int o = 0;
        for (const auto& i : inputs)
            o += i;
        outputs[0] = o == inputs.size();
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("And", as_gui);
    }
};