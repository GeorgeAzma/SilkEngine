#include "entry_point.h"
#include "core/my_app.h"

Application* createApplication(int argc, char** argv)
{
    return new MyApp({ argc, argv });
}