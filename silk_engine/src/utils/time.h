#pragma once

#include <chrono>

using namespace std::chrono_literals;

struct Time
{
public:
    Time() = default;

    template<typename Rep, typename Period>
    constexpr Time(const std::chrono::duration<Rep, Period>& duration) 
        : value(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) 
    {
    }

    static constexpr Time seconds(double seconds)
    {
        return Time(std::chrono::duration<double>(seconds));
    }

    static constexpr Time milliseconds(int32_t milliseconds)
    {
        return Time(std::chrono::duration<int32_t, std::micro>(milliseconds));
    }

    static constexpr Time microseconds(int64_t microseconds)
    {
        return Time(std::chrono::duration<int64_t, std::micro>(microseconds));
    }

    constexpr double asSeconds() const 
    {
        return value.count() * 0.000001;
    }

    constexpr int64_t asMicroseconds() const
    {
        return int64_t(value.count());
    }

    template<typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const 
    {
        return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(value);
    }

    constexpr bool operator==(const Time& rhs) const;
    constexpr bool operator!=(const Time& rhs) const;
    constexpr bool operator<(const Time& rhs) const;
    constexpr bool operator<=(const Time& rhs) const;
    constexpr bool operator>(const Time& rhs) const;
    constexpr bool operator>=(const Time& rhs) const;

    constexpr Time operator-() const;

    friend constexpr Time operator+(const Time& lhs, const Time& rhs);
    friend constexpr Time operator-(const Time& lhs, const Time& rhs);
    friend constexpr Time operator*(const Time& lhs, float rhs);
    friend constexpr Time operator*(const Time& lhs, int64_t rhs);
    friend constexpr Time operator*(float lhs, const Time& rhs);
    friend constexpr Time operator*(int64_t lhs, const Time& rhs);
    friend constexpr Time operator/(const Time& lhs, float rhs);
    friend constexpr Time operator/(const Time& lhs, int64_t rhs);
    friend constexpr double operator/(const Time& lhs, const Time& rhs);

    constexpr Time& operator+=(const Time& rhs);
    constexpr Time& operator-=(const Time& rhs);
    constexpr Time& operator*=(float rhs);
    constexpr Time& operator*=(int64_t rhs);
    constexpr Time& operator/=(float rhs);
    constexpr Time& operator/=(int64_t rhs);

private:
    std::chrono::microseconds value{};

public:
    static inline double dt = 0.0;
    static inline double runtime = 0.0;
    static inline unsigned int frame = 0;

public:
    //Gets time as double in seconds
    static double getTime()
    {
        return runtime;
    }

    static Time now() 
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
    }

    //Gets time as double in seconds, Slower alternative to getTime(), but this determines runtime without relying on the application
    static double getSystemTime()
    {
        return now().asSeconds();
    }
   
    static std::string getDateTime(const std::string& format = "%Y-%m-%d %H:%M:%S") 
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), format.c_str());
        return ss.str();
    }

private:
    static const inline std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
}; 






//Inline operator definitions
constexpr bool Time::operator==(const Time& rhs) const 
{
    return value == rhs.value;
}

constexpr bool Time::operator!=(const Time& rhs) const 
{
    return value != rhs.value;
}

constexpr bool Time::operator<(const Time& rhs) const
{
    return value < rhs.value;
}

constexpr bool Time::operator<=(const Time& rhs) const 
{
    return value <= rhs.value;
}

constexpr bool Time::operator>(const Time& rhs) const 
{
    return value > rhs.value;
}

constexpr bool Time::operator>=(const Time& rhs) const
{
    return value >= rhs.value;
}

constexpr Time Time::operator-() const 
{
    return Time(-value);
}

constexpr Time operator+(const Time& lhs, const Time& rhs) 
{
    return lhs.value + rhs.value;
}

constexpr Time operator-(const Time& lhs, const Time& rhs) 
{
    return lhs.value - rhs.value;
}

constexpr Time operator*(const Time& lhs, float rhs) 
{
    return lhs.value * rhs;
}

constexpr Time operator*(const Time& lhs, int64_t rhs) 
{
    return lhs.value * rhs;
}

constexpr Time operator*(float lhs, const Time& rhs) 
{
    return rhs * lhs;
}

constexpr Time operator*(int64_t lhs, const Time& rhs) 
{
    return rhs * lhs;
}

constexpr Time operator/(const Time& lhs, float rhs) 
{
    return lhs.value / rhs;
}

constexpr Time operator/(const Time& lhs, int64_t rhs)
{
    return lhs.value / rhs;
}

constexpr double operator/(const Time& lhs, const Time& rhs) 
{
    return static_cast<double>(lhs.value.count()) / static_cast<double>(rhs.value.count());
}

constexpr Time& Time::operator+=(const Time& rhs) 
{
    return *this = *this + rhs;
}

constexpr Time& Time::operator-=(const Time& rhs)
{
    return *this = *this - rhs;
}

constexpr Time& Time::operator*=(float rhs) 
{
    return *this = *this * rhs;
}

constexpr Time& Time::operator*=(int64_t rhs) 
{
    return *this = *this * rhs;
}

constexpr Time& Time::operator/=(float rhs) 
{
    return *this = *this / rhs;
}

constexpr Time& Time::operator/=(int64_t rhs)
{
    return *this = *this / rhs;
}
