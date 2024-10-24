
![Alt text](applicationScreenshot.png?raw=true "screenshot")


# About Mnemosy

Mnemosy is a tool to store and organize PBR texture files and materials.

- The goal of Mnemosy is to be fast, flexible and independent of any other software or service.
- This means that Mnemosy is designed to work with local files stored on your computer.
- No online accounts, services or subscriptions!
- Textures are only stored on one local machine and nowhere else.
- You can preview and organize your materials in a hierarchical folder structure.
- You can directly drag and drop all textures of a material to other programs with just one click.
- Or export your textures to a folder.
- Mnemosy also has build in channel packing support.


## Building from source
#### Prerequisites
- Windows 10/11
- CMake 3.25
- Visual Studio 17 2022

#### Build Steps
- Open command promt to a folder to build Mnemosy and run commands in order.
- Clone Repository: git clone https://github.com/itsFulcrum/Mnemosy.git
- Navigate to Mnemosy folder: cd Mnemosy
- Create a solution folder: mkdir Solution
- Generate build files: cmake -B Solution -G "Visual Studio 17 2022" 
- Build Debug: 	cmake --build Solution --target Mnemosy --config debug
- Or Release:	cmake --build Soluiton --target Mnemosy --config release
- Navigate to the Solution folder: cd Solution
- Run Mnemosy: MnemosyBuild\Release\Mnemosy.exe
