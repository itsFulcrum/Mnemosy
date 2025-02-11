cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)


# Command - Generate build files
# /Mnemosy: cmake -B Solution -G "Visual Studio 17 2022"

# Command - Build project:
# /Mnemosy/Solution: cmake --build . --target Mnemosy --config release
# /Mnemosy/Solution: cmake --build . --target Mnemosy --config debug

set(ENGINE_PROJECT "MnemosyEngine")
set(APP_PROJECT "Mnemosy")

project(${ENGINE_PROJECT})
project(${APP_PROJECT})


set(mnemosy_build_dir ${CMAKE_BINARY_DIR}/MnemosyBuild/)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${mnemosy_build_dir})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${mnemosy_build_dir})


# static everything please_build_dir})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/Resources DESTINATION ${mnemosy_build_dir}/)

include(GNUInstallDirs)
set(BUILD_SHARED_LIBS             	OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL	OFF CACHE BOOL "" FORCE)
set(USE_STATIC_CRT             	 	 ON CACHE BOOL "" FORCE)

add_compile_options(
          $<$<CONFIG:>:/MT>
          $<$<CONFIG:Debug>:/MTd>
          $<$<CONFIG:Release>:/MT>
)

# hide console window for release builds
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# ==== Load External libraries
# =============================================

set(DEPENDENCIES_DIR ${CMAKE_CURRENT_LIST_DIR}/Code/Dependencies)

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

#GLFW
set(glfw_subDir "Code/Dependencies/glfw-3.3.9")
include(${CMAKE_CURRENT_LIST_DIR}/${glfw_subDir}/cmake-Mnemosy-GlfwOptions.cmake)
add_subdirectory(${glfw_subDir})

# GLAD
set(glad_subDir "Code/Dependencies/glad")
add_subdirectory(${glad_subDir})


#GLM
set(glm_subDir "Code/Dependencies/glm/")
add_subdirectory(${glm_subDir})

# FULCRUM Utils
set(flcrm_subDir ${DEPENDENCIES_DIR}/FulcrumUtils/)
add_subdirectory(${flcrm_subDir})

# nlohmann Json
set(nlohmannJson_subDir ${DEPENDENCIES_DIR}/nlohmannjson)
add_subdirectory(${nlohmannJson_subDir})

## == Image libraries


# == ZLIB
set(zlib_subDir ${CMAKE_CURRENT_LIST_DIR}/Code/Dependencies/zlib-1.3.1)
add_subdirectory(${zlib_subDir})

SET(ZLIB_LIBRARY zlibstatic)
SET(ZLIB_ROOT ${zlib_subDir})

find_package(ZLIB REQUIRED)
message("MNEMOSY ZLIB Found "  ${ZLIB_FOUND})

# utf8cpp
set(utf8cpp_subDir ${DEPENDENCIES_DIR}/utf8cpp)
add_subdirectory(${utf8cpp_subDir})

# stbImage Read
set(stbImageRead_subDir ${DEPENDENCIES_DIR}/stbImageRead)
add_subdirectory(${stbImageRead_subDir})

# stbImage Write
set(stbImageWrite_subDir  ${DEPENDENCIES_DIR}/stbImageWrite)
add_subdirectory(${stbImageWrite_subDir})

# lodepng
set(lodepng_subDir ${DEPENDENCIES_DIR}/lodepng)
add_subdirectory(${lodepng_subDir})

# KTX
set(libktx_subDir ${DEPENDENCIES_DIR}/KTX-Software-main)
include(${libktx_subDir}/cmake-Mnemosy-LibKtxOptions.cmake)
add_subdirectory(${libktx_subDir})


#libTiff
set(libTiff_subDir ${DEPENDENCIES_DIR}/libtiff-master)
include(${libTiff_subDir}/cmake-Mnemosy_libTiff_options.cmake)
add_subdirectory(${libTiff_subDir})


#Open EXR
set(openExr_subDir ${DEPENDENCIES_DIR}/openexr-release)
include(${openExr_subDir}/cmake-Mnemosy_openExr_options.cmake)
add_subdirectory(${openExr_subDir})



#Assimp
set(assimp_subDir ${DEPENDENCIES_DIR}/assimp-master)
include(${assimp_subDir}/cmake-Mnemosy-AssimpOptions.cmake)
add_subdirectory(${assimp_subDir})



# ==== Load Internal Sources
# Setting ENGINE source files to: ENGINE_SOURCE_FILES
set(ENGINE_SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/Code/Engine)
include(${ENGINE_SOURCE_PATH}/cmake-EngineSourceFiles.cmake)

#ImGui
# Setting ImGui source files to: IMGUI_SOURCE_FILES
include(${DEPENDENCIES_DIR}/ImGui/cmake-ImGuiSourceFiles.cmake)
set(ImGui_Include_Dir ${DEPENDENCIES_DIR}/ImGui)

# Setting Application source files to: APP_SOURCE_FILES
set(APP_SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/Code/Application)
include(${APP_SOURCE_PATH}/cmake-ApplicationSourceFiles.cmake)



# === LINKING ===
# ===============
# include internal files
set(ALL_SOURCE_FILES ${ENGINE_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

add_library(${ENGINE_PROJECT} ${ALL_SOURCE_FILES})
target_include_directories(${ENGINE_PROJECT} PRIVATE ${ENGINE_SOURCE_PATH})

#target_include_directories(${ENGINE_PROJECT} PRIVATE ${nlohmannJson_subDir}/include/)

target_include_directories(${ENGINE_PROJECT} PRIVATE ${ImGui_Include_Dir})
target_include_directories(${ENGINE_PROJECT} PRIVATE ${zlib_subDir} ${CMAKE_CURRENT_BINARY_DIR}/Code/Dependencies/zlib-1.3.1)
target_include_directories(${ENGINE_PROJECT} PRIVATE ${assimp_subDir}/include/)

target_include_directories(${ENGINE_PROJECT} PRIVATE ${libktx_subDir}/lib/)



add_executable(${APP_PROJECT} ${APP_SOURCE_FILES})
target_include_directories(${APP_PROJECT} PRIVATE ${ENGINE_SOURCE_PATH})
target_include_directories(${APP_PROJECT} PRIVATE ${APP_SOURCE_PATH})
target_include_directories(${APP_PROJECT} PRIVATE ${ImGui_Include_Dir})



source_group(TREE ${APP_SOURCE_PATH} FILES ${APP_SOURCE_FILES})
source_group(TREE ${ENGINE_SOURCE_PATH} FILES ${ENGINE_SOURCE_FILES})


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_PROJECT})


# public for libraries that application also needs
target_link_libraries(${ENGINE_PROJECT} PUBLIC
glfw
glm
nlohmann_json
Fulcrum_Utils
utf8cpp
)


target_link_libraries(${ENGINE_PROJECT} PRIVATE
OpenGL::GL
glad

zlibstatic
tiff
lodepng
ktx
OpenEXR
stbImageWrite
stbImageRead
assimp
)

target_link_libraries(${APP_PROJECT} PRIVATE ${ENGINE_PROJECT} )
