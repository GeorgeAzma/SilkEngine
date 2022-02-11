#pragma once

#include <chrono>

struct Time
{
public:
    static inline double dt = 0.0;
    static inline double runtime = 0.0;
    static inline unsigned int frame = 0;

public:
    static double getTime()
    {
        return runtime;
    }
    //Slower function which determines runtime without relying on the application, otherwise prefer getTime()
    static double getSystemTime()
    {
        return (std::chrono::high_resolution_clock::now().time_since_epoch().count() - start) * 0.000000001;
    }

private:
    static const inline long long start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}; 