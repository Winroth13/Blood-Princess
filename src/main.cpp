#include <iostream>

#include "core/application.hpp"

int main()
{
	Application app;

	if (!app.Initialize()) 
		return 1;

	app.Run();
	app.Shutdown();

	return 0;
}