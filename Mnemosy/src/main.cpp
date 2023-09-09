
#include <controller/App.h>


int main() {
	App* application = new App();

	application->run();


	delete application;
	return 0;
}

