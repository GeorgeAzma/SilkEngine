#pragma once

#include "application.h"

extern Application *createApp(ApplicationCommandLineArgs args);

int main(int argc, char **argv)
{
    auto app = createApp({ argc, argv });
    app->run();
    delete app;

    return EXIT_SUCCESS;
}