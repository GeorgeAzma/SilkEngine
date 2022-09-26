#pragma once

#include "gate.h"
#include "gfx/renderer.h"

class Or : public Gate
{
public:
    Or() : Gate(2, 1) {}

    Gate* clone() const override{ return new Or(*this); }
    operator size_t() const override { return ID<Or>(); }

    void simulateLogic() override
    {
        unsigned int o = 0;
        for (const auto& i : inputs)
            o += i;
        outputs[0] = o > 0;
    }

    void render(bool as_gui = false) const override
    {
        renderGeneric("Or", as_gui);
    }
};