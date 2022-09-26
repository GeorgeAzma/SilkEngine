#pragma once

template <typename T>
struct Delta
{
public:
    Delta(const T &old)
        : old{old}, now{old} {}

    T operator()(const T& new_val)
    {
        old = now;
        now = new_val;
        return delta = now - old;
    }

    operator T() const { return delta; }

private:
    T delta{};
    T old{};
    T now{};
};