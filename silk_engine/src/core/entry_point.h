#pragma once

#include <iostream>
#include "application.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

extern Application *createApp(ApplicationCommandLineArgs args);

int main(int argc, char **argv)
{
    auto app = createApp({ argc, argv });
    app->run();
    Graphics::logical_device->waitIdle();
    delete app;

    return EXIT_SUCCESS;
}