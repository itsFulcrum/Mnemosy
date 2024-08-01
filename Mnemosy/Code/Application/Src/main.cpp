#include "Include/Application.h"

int main() {

	mnemosy::Application& app = mnemosy::Application::GetInstance();
	app.Initialize();
	app.Run();
	app.Shutdown();

	return 0;
}