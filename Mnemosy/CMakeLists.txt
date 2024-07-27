cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)

set(ENGINE_PROJECT "MnemosyEngine")
set(APP_PROJECT "Mnemosy")

project(${ENGINE_PROJECT})
project(${APP_PROJECT})


set(mnemosy_build_dir ${CMAKE_BINARY_DIR}/MnemosyBuild/)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${mnemosy_build_dir})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${mnemosy_build_dir})


include(GNUInstallDirs)

# ==== Load External libraries
# =============================================
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

# SpdLog
set(spdLog_subDir "Code/Dependencies/spdlog-1.x/")
include(${CMAKE_CURRENT_LIST_DIR}/${spdLog_subDir}cmake-Mnemosy-SpdLogOptions.cmake)
add_subdirectory(${spdLog_subDir})

#stbImage
set(stbImage_subDir "Code/Dependencies/stbImage/")
add_subdirectory(${stbImage_subDir})

#GLM
set(glm_subDir "Code/Dependencies/glm/")
add_subdirectory(${glm_subDir})

#Assimp
set(assimp_subDir "Code/Dependencies/assimp-master")
include(${CMAKE_CURRENT_LIST_DIR}/${assimp_subDir}/cmake-Mnemosy-AssimpOptions.cmake)
add_subdirectory(${assimp_subDir})

# KTX
set(ktx_software_subDir "Code/Dependencies/KTX-Software-main/")
include(${CMAKE_CURRENT_LIST_DIR}/${ktx_software_subDir}cmake-Mnemosy-LibKtxOptions.cmake)
add_subdirectory(${ktx_software_subDir})

# Json
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(nlohmann_json_subDir "Code/Dependencies/json-3.9.1/")
add_subdirectory(${nlohmann_json_subDir})

## OPEN CV
include(${CMAKE_CURRENT_LIST_DIR}/Code/Dependencies/openCV/cmake-Mnemosy_OpenCVOptions.cmake)
set(OpenCV_DIR "Code/Dependencies/openCV/opencvBuild/install/")
find_package(OpenCV REQUIRED PATHS "Code/Dependencies/openCV/opencvBuild/install/x64/vc17/bin/")

# copy open cv dlls into the build directory
file(MAKE_DIRECTORY ${mnemosy_build_dir})
file(MAKE_DIRECTORY ${mnemosy_build_dir}/Debug)
file(MAKE_DIRECTORY ${mnemosy_build_dir}/Release)
# Copy debug/release dlls to respective executable directory
configure_file("${_OpenCV_LIB_PATH}/opencv_core490d.dll"        ${mnemosy_build_dir}/Debug/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgcodecs490d.dll"   ${mnemosy_build_dir}/Debug/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgproc490d.dll"     ${mnemosy_build_dir}/Debug/ COPYONLY)

configure_file("${_OpenCV_LIB_PATH}/opencv_core490.dll"         ${mnemosy_build_dir}/Release/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgcodecs490.dll"    ${mnemosy_build_dir}/Release/ COPYONLY)
configure_file("${_OpenCV_LIB_PATH}/opencv_imgproc490.dll"      ${mnemosy_build_dir}/Release/ COPYONLY)

# ==== Load Internal Sources
# Setting ENGINE source files to: ENGINE_SOURCE_FILES
set(ENGINE_SOURCE_PATH Code/Engine)
include(${CMAKE_CURRENT_LIST_DIR}/${ENGINE_SOURCE_PATH}/cmake-EngineSourceFiles.cmake)

#ImGui
# Setting ImGui source files to: IMGUI_SOURCE_FILES
include(${CMAKE_CURRENT_LIST_DIR}/Code/Engine/External/ImGui/cmake-ImGuiSourceFiles.cmake)

# Setting Application source files to: APP_SOURCE_FILES
set(APP_SOURCE_PATH Code/Application)
include(${CMAKE_CURRENT_LIST_DIR}/Code/Application/cmake-ApplicationSourceFiles.cmake)


# === LINKING ===
# ===============
# include internal files
set(ALL_SOURCE_FILES ${ENGINE_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

add_library(${ENGINE_PROJECT} ${ALL_SOURCE_FILES})
target_include_directories(${ENGINE_PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Code/Engine)

add_executable(${APP_PROJECT} ${APP_SOURCE_FILES})
target_include_directories(${APP_PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Code/Engine)
target_include_directories(${APP_PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Code/Application)



set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_PROJECT})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/${APP_SOURCE_PATH} FILES ${APP_SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/${ENGINE_SOURCE_PATH} FILES ${ALL_SOURCE_FILES})

# include external librariy headers
target_include_directories(${ENGINE_PROJECT} PRIVATE libs/assimp-src/include)
target_include_directories(${ENGINE_PROJECT} PRIVATE ${ktx_software_subDir}lib/)
target_include_directories(${ENGINE_PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Code/Dependencies/openCV/opencvBuild/install/include)
target_include_directories(${APP_PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Code/Dependencies/openCV/opencvBuild/install/include)
# public for libraries that application also needs
target_link_libraries(${ENGINE_PROJECT} PUBLIC
glfw
glm
nlohmann_json::nlohmann_json
spdlog::spdlog_header_only
)

target_link_libraries(${ENGINE_PROJECT} PRIVATE
OpenGL::GL
glad
assimp
stbImage
ktx
${OpenCV_LIBS}
)

target_link_libraries(${APP_PROJECT} PRIVATE ${ENGINE_PROJECT} )
