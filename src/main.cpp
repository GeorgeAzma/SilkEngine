#include "silk_engine/entry_point.h"
#include "core/my_app.h"

Application* createApplication()
{
    return new MyApp();
}