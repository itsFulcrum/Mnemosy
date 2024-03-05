#include "Include/Application.h"

// seems to always produce memory leak report even if main does literaly nothing and nothing is includuded
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


int main() {

	mnemosy::Application& app = mnemosy::Application::GetInstance();
	app.Initialize();
	app.Run();
	app.Shutdown();

	return 0;
}