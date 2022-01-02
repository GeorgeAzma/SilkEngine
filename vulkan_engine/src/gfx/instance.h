#pragma once

struct AppInfo
{
    const char *name = "App";
    unsigned int version = 0;
};

class Instance
{
public:
    Instance(const AppInfo &app_info = {});

private:
};