#pragma once

template <typename T>
class Detected
{
public:
    Detected(const T& value, const std::function<void()>& on_value_changed)
        : value(value), on_value_changed(on_value_changed) {}

    Detected<T>& operator=(const T& other)
    {
        if constexpr (std::equality_comparable<T>)
            if (value == other)
                return *this;
        value = other;
        on_value_changed();

        return *this;
    }

    Detected<T>& operator=(const Detected<T>& other)
    {
        if constexpr (std::equality_comparable<T>)
            if (value == other.value)
                return *this;
        value = other.value;
        on_value_changed();

        return *this;
    }

    operator const T& () const { return value; }

private:
    T value;
    std::function<void()> on_value_changed;
};