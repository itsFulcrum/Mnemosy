#  Mnemosy

Currently in development nothing fancy yet


the plan for this project is to become a texture library tool similar to Quixel but offline without any subscription.  Essentially a handy tool to manage and organize textures you already have on disk with cool preview and handy conversion and channel packing features. 

It is also  a learning project for me and my first bigger project using c++ and generally a way for me to learn and understand graphics programming and the graphics pipeline better


## Building the project

- In Mnemosy/  make a Solution folder for the build
- Using CMake , build Mnemosy/ into Mnemosy/Solution
- Run configure and Generate in cmake 
- Now go to Mnemosy/Solution/bin and delete the two files "Debug" and "Release"
- Now Open MnemosyApplication.sln if you were building for vs 
- Build the Solution once for debug and for release
-  Before you can run the project you need to Configure and Run CMake one more time because I use cmke to copy some dll's from OpenCV into the bin/Debug/ folder
- After that you should be run the application

currently the install process is a bit weird and with the dlls from openCV hopefully i can fix that somehow.   It Can also be that the dlls from OpenCV in this repo do not work on your system.  In That case you'd have to build OpenCV yourself and put the dlls and .lib files you build into the following folder:
Mnemosy/Code/Dependencies/openCV/opencvBuild/install/x64/vc17/