#pragma once

#include "core/application.h"

extern Application* createApplication();

int Main(int argc, char** argv)
{
	Log::init();
	Application::command_line_args = { argc, argv };
	Application* app = createApplication();
	SK_ASSERT(app, "Client Application is null!");
	app->run();
	delete app;

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
