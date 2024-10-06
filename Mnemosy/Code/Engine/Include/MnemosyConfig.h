#ifndef MNEMOSY_CONFIG_H
#define MNEMOSY_CONFIG_H

#define MNEMOSY_VERSION_MAJOR 0
#define MNEMOSY_VERSION_MINOR 7
#define MNEMOSY_VERSION_SUFFIX "alpha"

#define MNEMOSY_CONFIG_DEBUG
//#define MNEMOSY_CONFIG_RELEASE


// only tested on widows so far
#define MNEMOSY_PLATFORM_WINDOWS
//#define MNEMOSY_PLATFORM_LINUX


#define MNEMOSY_CONFIG_ENABLE_VSYNC
//#define MNEMOSY_RENDER_GIZMO


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
