#include "my_app.h"
#include "my_scene.h"
#include "scene/scene_manager.h"

MyApp::MyApp(ApplicationCommandLineArgs args)
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

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new MyApp(args);
}
