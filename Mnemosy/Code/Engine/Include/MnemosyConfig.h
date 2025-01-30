#ifndef MNEMOSY_CONFIG_H
#define MNEMOSY_CONFIG_H

#define MNEMOSY_VERSION_MAJOR 1
#define MNEMOSY_VERSION_MINOR 0
#define MNEMOSY_VERSION_SUFFIX "beta"

#define MNEMOSY_CONFIG_DEBUG
//#define MNEMOSY_CONFIG_RELEASE


// works only on windows atm // mostly because of file dialogs, default library folder location & drag and drop
#define MNEMOSY_PLATFORM_WINDOWS
//#define MNEMOSY_PLATFORM_LINUX


//#define MNEMOSY_CONFIG_DISABLE_VSYNC
//#define MNEMOSY_RENDER_GIZMO
#define MNSY_CONFIG_CAP_DELTA_TIME



#define MNEMOSY_SRC_WINDOW_WIDTH  1400;
#define MNEMOSY_SRC_WIDNOW_HEIGHT 850;


// all namespaces
namespace mnemosy
{
	namespace core
	{}
	namespace graphics
	{}
	namespace systems
	{}
	namespace gui
	{}
}



#endif // !MNEMOSY_CONFIG_H
