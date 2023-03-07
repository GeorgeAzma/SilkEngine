#include "core/my_app.h"

int main(int argc, char** argv)
{
    auto app = new MyApp({ argc, argv });
    app->run();
    delete app;
    return EXIT_SUCCESS;
}