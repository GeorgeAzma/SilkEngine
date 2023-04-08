#include "silk_engine/scene/scene_manager.h"

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp(const ApplicationCommandLineArgs& args)
{
    scene = makeShared<MyScene>();
    SceneManager::add(scene);
}

void MyApp::onUpdate()
{
}

MyApp::~MyApp()
{
    SceneManager::destroy();
}