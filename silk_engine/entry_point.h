#pragma once

#include "core/application.h"
#include "gfx/window/glfw.h"
#include "core/input/input.h"
#include "gfx/render_context.h"
#include "scene/resources.h"

extern Application* createApplication(int argc, char** argv);

int Main(int argc, char** argv)
{
	std::string app_name = "My App";

	Log::init();
	SK_INFO("Started");
	GLFW::init();
	Input::init();
	RenderContext::init(app_name);
	Resources::init();

	Application* app = createApplication(argc, argv);
	SK_ASSERT(app, "Client Application is null!");
	app->run();
	delete app;

	GLFW::destroy();
	Resources::destroy();
	RenderContext::destroy();
	SK_INFO("Terminated");

	return 0;
}


#if defined(SK_DIST) && defined(SK_PLATFORM_WINDOWS)
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Main(__argc, __argv);
}
#else
int main(int argc, char** argv)
{
	return Main(argc, argv);
}
#endif
