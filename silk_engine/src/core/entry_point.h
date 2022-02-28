#pragma once

#include <iostream>
#include "application.h"
#include "gfx/devices/logical_device.h"

extern Application *createApp(ApplicationCommandLineArgs args);

int main(int argc, char **argv)
{
    try
    {
        auto app = createApp({ argc, argv });
        app->run();
        Graphics::logical_device->waitIdle();
        delete app;
    }
    catch (const vk::SystemError& err) 
    {
        SK_ERROR(err.what());
    }
    catch (const std::exception &e)
    {
        SK_ERROR(e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        SK_ERROR("Unknown error has occurred");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}