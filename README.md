
![Alt text](\Mnemosy/Resources/Textures/ApplicationScreenshot.png?raw=true "screenshot")

#  About Mnemosy

Mnemosy is a tool to store and organize PBR texture files and materials.

The goal of Mnemosy is to be fast, flexible and independent of any other software or service.
This means that Mnemosy is designed to work with local files stored on your computer. No online accounts, services or subscriptions!
Textures are only stored on this local machine and nowhere else.
Mnemosy lets you export textures associated with a material in common image formats like .png or .tif
it even lets you drag and drop them directly from within the app to other software that implement drag and drop

It is also  a learning project for me and my first bigger project using c++ and generally a way for me to learn and understand graphics programming and the graphics pipeline better


## Roadmap

- Propper support for non square images
- Fix Bugs when importing .jpg files
- Support for .exr Image files
- Make copying entire library folder to other systems easier
- Export dummy textures
- Hight map support
- Transparency texture support
- Convert Glossy/Smoothness to Roughness map feature
- Channel packing system
- Better camera controller
- SSAO
- Post Processing pass
- Material Search engine
- Performance Optimization at load time. (Propper mesh registry.)
- **skybox shader blurring** (propper blurring algorinthm.. )
- **improve irradiance map generation, artifacts on some cubemaps
	**look into "probability distribution funktions" (pdfs) for importance sampling


## Building from source

- In Mnemosy/  make a Solution folder for the build
- Using CMake , build Mnemosy/ into Mnemosy/Solution
- Run configure and Generate in cmake
- Now go to Mnemosy/Solution/bin and delete the two files "Debug" and "Release"
- Now Open MnemosyApplication.sln if you were building for VS
- Build the Solution once for debug and for release
-  Before you can run the project you need to Configure and Run CMake one more time because I use cmke to copy some dll's from OpenCV into the bin/Debug/ folder
- After that you should be able run the application

currently the install process is a bit weird and with the dlls from openCV hopefully i can fix that somehow.
