#pragma once

#include "core/application.h"

extern Application* createApplication();

int Main(int argc, char** argv)
{
	try
	{
		Log::init();
		Application::command_line_args = { argc, argv };
		Application* app = createApplication();
		if (app)
		{
			app->run();
			delete app;
		}
	}
	catch (const std::exception& e)
	{
		SK_CRITICAL("Exception: {}", e.what());
	}
	return 0;
}


#if defined(SK_DIST) && defined(SK_PLATFORM_WINDOWS)
#include <Windows.h>
#undef min
#undef max
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
