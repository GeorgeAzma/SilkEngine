#pragma once

#include <iostream>
#include "application.h"

extern Application *createApp(ApplicationCommandLineArgs args);

int main(int argc, char **argv)
{
    try
    {
        auto app = createApp({argc, argv});
        app->run();
        Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));
        delete app;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}