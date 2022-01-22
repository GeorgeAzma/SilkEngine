#pragma once

template <typename T>
struct Delta
{
public:
    Delta(const T &old)
        : old{old}, now{old} {}

    bool update(const T & new_val)
    {
        return calc(new_val) != 0;
    }

    T calc(const T & new_val)
    {
        old = now;
        now = new_val;
        return delta = now - old;
    }

    T operator()(const T& new_val)
    {
        return calc(new_val);
    }

    constexpr operator T() const { return delta; }

private:
    T delta = {};
    T old = {};
    T now = {};
};