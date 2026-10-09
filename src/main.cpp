#include <iostream>

#include "core/application.hpp"


// Hint to make the application use dedicated graphics cards (NIVIDA/AMD)
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}

int main()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Application app;

	if (!app.Initialize()) 
		return 1;

	app.Run();
	app.Shutdown();

	return 0;
}
