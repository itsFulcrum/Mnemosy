#ifndef MNEMOSY_CONFIG_H
#define MNEMOSY_CONFIG_H

#define MNEMOSY_VERSION_MAJOR 0
#define MNEMOSY_VERSION_MINOR 1

#define MNEMOSY_CONFIG_DEBUG
//#define MNEMOSY_CONFIG_RELEASE


// only tested on widows so far
#define MNEMOSY_PLATFORM_WINDOWS
//#define MNEMOSY_PLATFORM_LINUX


#define MNEMOSY_CONFIG_ENABLE_VSYNC



static unsigned int SRC_WINDOW_WIDTH = 1600;
static unsigned int SRC_WIDNOW_HEIGHT = 1000;


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