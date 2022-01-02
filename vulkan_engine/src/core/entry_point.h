#include <iostream>
extern Application *createApp();

int main(int argc, char **argv)
{
    try
    {
        auto app = createApp();
        app->run();
        delete app;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}