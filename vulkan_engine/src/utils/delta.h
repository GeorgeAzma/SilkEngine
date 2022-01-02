#pragma once

template <typename T>
struct Delta
{
public:
    Delta(const T &old)
        : old{old}, now{old}, delta(0) {}

    bool update(const T &newVal)
    {
        return calc(newVal) != 0;
    }

    T calc(const T &newVal)
    {
        old = now;
        now = newVal;
        return delta = now - old;
    }

    operator T() const { return delta; }

private:
    T delta = 0;
    T old = 0;
    T now = 0;
};