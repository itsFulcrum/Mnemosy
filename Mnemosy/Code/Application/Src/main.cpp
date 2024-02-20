//#include "Engine/Include/MnemosyEngine.h"
#include "Application/Include/Application.h"

int main() {
	
	mnemosy::Application& app = mnemosy::Application::GetInstance();

	app.Initialize();
	app.Run();
	app.Shutdown();

	//delete app;
	//app = nullptr;

	return 0;
}