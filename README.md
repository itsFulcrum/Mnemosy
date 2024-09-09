
![Alt text](Mnemosy/Resources/Textures/applicationScreenshot.png?raw=true "screenshot")


#  About Mnemosy

Mnemosy is a tool to store and organize PBR texture files and materials.

- The goal of Mnemosy is to be fast, flexible and independent of any other software or service.
- This means that Mnemosy is designed to work with local files stored on your computer.
- No online accounts, services or subscriptions!
- Textures are only stored on one local machine and nowhere else.
- You can preview and organize your materials in a hierarchical folder structure.
- You can directly drag and drop all textures of a material to other programs with just one click.
- Or export your textures to a folder.
- Mnemosy also has build in channel packing support.


## Roadmap

- Presets for channel packing
- Store more user settings
- Support multi selection for folders
- Support .exr files for and skybox images
- Store skybox meta data (rendering settings)
- Possiblity to load and entire folder structure into menemosy and try to automatically detect textures that form one material based on their names

## Building from source
#### Prerequisites
- CMake
- Visual Studio 17

#### Build Steps
- Clone the repository 'https://github.com/itsFulcrum/Mnemosy.git'
- Create a solution folder in  Mnemosy/Solution
- Run CMake from the folder with the main CMakeLists.txt ( Mnemosy/CMakeLists.txt ) into your created solution folder and create a visual studio solution.
- Open the solution Mnemosy.sln
- Right click the visual studio project called Mnemosy and go into properties->advanced and change "Copy C++ Runtime to OutDir" to yes
- Build the project for debug and/or release

- You can now execute <SolutionFolder>/MnemosyBuild/Release/Mnemosy.exe