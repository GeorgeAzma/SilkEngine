#pragma once

struct Time
{
public:
    static void update()
    {
        double new_runtime = getHighResTime() - Time::start;
        dt = new_runtime - runtime;
        runtime = new_runtime;
        ++frame;
    }

    //Gets time as double in seconds
    static double getTime()
    {
        return runtime;
    }

    static double getTimeSinceEpoch()
    {
        return start + runtime;
    }

    static double getHighResTime()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    static double getUnixTime()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static double getFileTime()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::file_clock::now().time_since_epoch()).count();
    }
   
    static std::string getDateTime(std::string_view format = "%Y-%m-%d %H:%M:%S") 
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), format.data());
        return ss.str();
    }

public:
    static inline const double start = getHighResTime();
    static inline const double unix_start = getUnixTime();
    static inline const double file_start = getFileTime();
    static inline double dt = 0.0;
    static inline double runtime = 0.0;
    static inline uint64_t frame = 0;
};