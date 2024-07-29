
![Alt text](Mnemosy/Resources/Textures/appScreenshot.png?raw=true "screenshot")


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

- Export of Dummy textures if no texture is assigned, using the value of the material
- Material search function
- Presets for channel packing
- Store more user settings
- Support multi selection for folders
- Possibly add rename to material multi selection (not fixed yet)
- Support .exr files for both textures and skybox images
- Store skybox meta data (rendering settings)
- Allow Dragging multiple files into material editor or viewport and it should try to match the filenames to a texture slot
- Story Library Data alongside the folder to make copying entire libraries to different systems easier
- Possiblity to load and entire folder structure into menemosy and try to automatically detect textures that form one material based on their names

## Building from source
#### Prerequisites
- CMake
- Visual Studio 17

- Clone the repository
- Create a solution folder in  Mnemosy/Solution
- Run CMake from the folder with the main CMakeLists.txt ( Mnemosy/CMakeLists.txt ) into your created solution folder and create a visual studio solution.
- Open the solution Mnemosy.sln
- Right click the visual studio project called Mnemosy and go into properties->advanced and change "Copy C++ Runtime to OutDir" to yes
- Build the project for debug and/or release
- Now you can run Mnemosy through visual studio

- To create a final build, first, create a new folder. Within create another folder called "bin" (anything works) and copy all contents including the Mnemosy.exe and all .dll's that were build to /SolutionFolder/MnemosyBuild/Release/ into this "bin" folder.
- Now copy the "Resources" folder from the repository (/Mnemosy/Resources) next to the bin Folder.
- The Folder Structure should look like this
- BuildFolder
	- Bin
		- Mnemosy.exe
		- ...
	- Resources
		- Data
		- ...

- Mnemosy can now be run from that Mnemosy.exe
