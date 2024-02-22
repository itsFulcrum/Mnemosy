cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)

set(ENGINE_PROJECT "MnemosyEngine")
set(APP_PROJECT "MnemosyApplication")

project(${ENGINE_PROJECT})
project(${APP_PROJECT})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#install(FILES $<TARGET_RUNTIME_DLLS:${APP_PROJECT}> TYPE BIN)

IF (WIN32)

ELSE()

ENDIF()


#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

#GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(glfw_subDir "Code/Dependencies/glfw-3.3.9/")
add_subdirectory(${glfw_subDir})

# GLAD
set(glad_subDir "Code/Dependencies/glad/")
add_subdirectory(${glad_subDir})

# SpdLog

set(SPDLOG_SYSTEM_INCLUDES ON CACHE BOOL "" FORCE)
set(SPDLOG_TIDY ON CACHE BOOL "" FORCE)
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(spdLog_subDir "Code/Dependencies/spdlog-1.x/")
add_subdirectory(${spdLog_subDir})


#stbImage
set(stbImage_subDir "Code/Dependencies/stbImage/")
add_subdirectory(${stbImage_subDir})

#GLM
set(glm_subDir "Code/Dependencies/glm/")
add_subdirectory(${glm_subDir})

#Assimp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(assimp_subDir "Code/Dependencies/assimp-master/")
add_subdirectory(${assimp_subDir})


#ImGui
set(imGui_subDir "Code/Engine/External/ImGui")
#add_subdirectory(${imGui_Dir})
set(IMGUI_SOURCE_FILES
${imGui_subDir}/imconfig.h
${imGui_subDir}/imgui.h
${imGui_subDir}/imgui_internal.h
${imGui_subDir}/imstb_rectpack.h
${imGui_subDir}/imstb_textedit.h
${imGui_subDir}/imstb_truetype.h
${imGui_subDir}/imgui.cpp
${imGui_subDir}/imgui_draw.cpp
${imGui_subDir}/imgui_tables.cpp
${imGui_subDir}/imgui_widgets.cpp
#Demo file
${imGui_subDir}/imgui_demo.cpp

#backends
${imGui_subDir}/imgui_impl_glfw.h
${imGui_subDir}/imgui_impl_glfw.cpp
${imGui_subDir}/imgui_impl_opengl3.h
${imGui_subDir}/imgui_impl_opengl3.cpp
${imGui_subDir}/imgui_impl_opengl3_loader.h

${imGui_subDir}/imgui_stdlib.h
${imGui_subDir}/imgui_stdlib.cpp
)


# Json
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(nlohmann_json_subDir "Code/Dependencies/json-3.9.1/")
add_subdirectory(${nlohmann_json_subDir})

#Not Using Anymore implmented as header file directly
##shadinclude
#set(shadinclude_subDir "Code/Dependencies/shadinclude/")
#add_subdirectory(${shadinclude_subDir})

#tiny xml
set(tinyxml2_subDir "Code/Dependencies/tinyxml2/")
add_subdirectory(${tinyxml2_subDir})


## OPEN CV
#set(opencv_subDir "Code/Dependencies/openCV/opencv/")
#add_subdirectory(${opencv_subDir})

set(BUILD_opencv_apps OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_calib3d OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_dnn OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_features2d OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_flann OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_gapi OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_highgui OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_java OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_java_bindings_generator OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_js OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_js_bindings_generator OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_ml OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_obj_bindings_generator ON CACHE BOOL "" FORCE)
set(BUILD_opencv_objdetect OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_photo OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python_bindings_generator OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python_tests OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_stitching OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_video OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_videoio OFF CACHE BOOL "" FORCE)
set(PARALLEL_ENABLE_PLUGINS OFF CACHE BOOL "" FORCE)
set(OpenCV_DIR "Code/Dependencies/openCV/opencvBuild/install/")
find_package(OpenCV REQUIRED PATHS "Code/Dependencies/openCV/opencvBuild/install/x64/vc17/bin/")

# copy dlls to executable directory
configure_file("${_OpenCV_LIB_PATH}/opencv_core490d.dll"      ${CMAKE_BINARY_DIR}/bin/Debug/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgcodecs490d.dll" ${CMAKE_BINARY_DIR}/bin/Debug/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgproc490d.dll" ${CMAKE_BINARY_DIR}/bin/Debug/ COPYONLY)

configure_file("${_OpenCV_LIB_PATH}/opencv_core490.dll"      ${CMAKE_BINARY_DIR}/bin/Release/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgcodecs490.dll"  ${CMAKE_BINARY_DIR}/bin/Release/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgproc490.dll" ${CMAKE_BINARY_DIR}/bin/Release/ COPYONLY)



include_directories(${CMAKE_CURRENT_LIST_DIR}/Code)

set(ENGINE_SOURCE_PATH Code/Engine)
set(ENGINE_SOURCE_FILES
# Mnemosy engine source files
${ENGINE_SOURCE_PATH}/Include/MnemosyConfig.h
${ENGINE_SOURCE_PATH}/Include/MnemosyEngine.h
${ENGINE_SOURCE_PATH}/Src/MnemosyEngine.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Window.h
${ENGINE_SOURCE_PATH}/Src/Core/Window.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Logger.h
${ENGINE_SOURCE_PATH}/Src/Core/Logger.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Log.h
${ENGINE_SOURCE_PATH}/Include/Core/Clock.h
${ENGINE_SOURCE_PATH}/Src/Core/Clock.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/Input/InputSystem.h
${ENGINE_SOURCE_PATH}/Src/Systems/Input/InputSystem.cpp

#Utils
${ENGINE_SOURCE_PATH}/Include/Core/Utils/PlatfromUtils_Windows.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/PlatfromUtils_Windows.cpp


# Graphics
${ENGINE_SOURCE_PATH}/Include/Graphics/Utils/ShaderIncludeParser.h

${ENGINE_SOURCE_PATH}/Include/Graphics/Shader.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Shader.cpp

${ENGINE_SOURCE_PATH}/include/Graphics/MeshData.h
${ENGINE_SOURCE_PATH}/include/Graphics/ModelData.h
${ENGINE_SOURCE_PATH}/include/Graphics/ModelLoader.h
${ENGINE_SOURCE_PATH}/Src/Graphics/ModelLoader.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Texture.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Texture.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Material.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Material.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Camera.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Camera.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Image.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Image.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Cubemap.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Cubemap.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/ImageBasedLightingRenderer.h
${ENGINE_SOURCE_PATH}/Src/Graphics/ImageBasedLightingRenderer.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Transform.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Transform.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/RenderMesh.h
${ENGINE_SOURCE_PATH}/Src/Graphics/RenderMesh.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Renderer.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Renderer.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Skybox.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Skybox.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Light.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Light.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/Scene.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Scene.cpp

#user interface
${ENGINE_SOURCE_PATH}/Include/Gui/UserInterface.h
${ENGINE_SOURCE_PATH}/Src/Gui/UserInterface.cpp
${ENGINE_SOURCE_PATH}/Include/Gui/GuiPanel.h
${ENGINE_SOURCE_PATH}/Src/Gui/GuiPanel.cpp
)


set(APP_SOURCE_PATH Code/Application)
set(APP_SOURCE_FILES
# Mnemosy application source files
#include
${APP_SOURCE_PATH}/Include/Application.h
${APP_SOURCE_PATH}/Include/Input/CameraInputController.h
${APP_SOURCE_PATH}/Include/Input/SceneInputController.h


#Src
${APP_SOURCE_PATH}/Src/main.cpp
${APP_SOURCE_PATH}/Src/Application.cpp
${APP_SOURCE_PATH}/Src/Input/CameraInputController.cpp
${APP_SOURCE_PATH}/Src/Input/SceneInputController.cpp

#gui panels
${APP_SOURCE_PATH}/Include/GuiPanels/GuiPanelManager.h
${APP_SOURCE_PATH}/Src/GuiPanels/GuiPanelManager.cpp

${APP_SOURCE_PATH}/Include/GuiPanels/MainMenuBarGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/MainMenuBarGuiPanel.cpp
${APP_SOURCE_PATH}/Include/GuiPanels/GlobalSettingsGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/GlobalSettingsGuiPanel.cpp

${APP_SOURCE_PATH}/Include/GuiPanels/ViewportGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/ViewportGuiPanel.cpp
${APP_SOURCE_PATH}/Include/GuiPanels/SceneSettingsGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/SceneSettingsGuiPanel.cpp
${APP_SOURCE_PATH}/Include/GuiPanels/MaterialEditorGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/MaterialEditorGuiPanel.cpp

)


# linking
#could split up source files by namespaces in their folders or something like that
set(ALL_SOURCE_FILES ${ENGINE_SOURCE_FILES} ${IMGUI_SOURCE_FILES})
add_library(${ENGINE_PROJECT} ${ALL_SOURCE_FILES})

add_executable(${APP_PROJECT} ${APP_SOURCE_FILES})


#install(FILES $<TARGET_RUNTIME_DLLS:${APP_PROJECT}> TYPE BIN)

#add_custom_command(TARGET ${APP_PROJECT} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${OpenCV_DIR}x64/vc17/bin/"
#        $<TARGET_FILE_DIR:${APP_PROJECT})

#${OpenCV_DIR}x64/vc17/bin/
#COPY_DLL(${APP_PROJECT} opencv_core490d.dll)
#COPY_DLL(${APP_PROJECT} opencv_imgproc490d.dll)



set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_PROJECT})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/${APP_SOURCE_PATH} FILES ${APP_SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/${ENGINE_SOURCE_PATH} FILES ${ALL_SOURCE_FILES})

# link external libraries
target_include_directories(${ENGINE_PROJECT} PRIVATE libs/assimp-src/include)




target_link_libraries(${ENGINE_PROJECT} PUBLIC
OpenGL::GL
glad
glfw
glm
#imGui
assimp
stbImage
##shadinclude
nlohmann_json::nlohmann_json
tinyxml2
)

target_link_libraries(${ENGINE_PROJECT} PUBLIC ${OpenCV_LIBS})


#target_link_libraries(${ENGINE_PROJECT} PUBLIC opencv)


target_link_libraries(${ENGINE_PROJECT} PRIVATE spdlog::spdlog_header_only)
#target_link_libraries(${ENGINE_PROJECT} PUBLIC imGui)

#target_link_libraries(${ENGINE_PROJECT} spdlog)

target_link_libraries(${APP_PROJECT} ${ENGINE_PROJECT} )
