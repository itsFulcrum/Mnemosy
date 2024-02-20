cmake_minimum_required(VERSION 3.14)
project(imgui)



# link imGui

set(IMGUI_NATVIS_FILE "imgui.natvis")
set(IMGUI_NATVIS_STEP_FILTER_FILE "imgui.natstepfilter")


set(IMGUI_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imconfig.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui_internal.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imstb_rectpack.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imstb_textedit.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imstb_truetype.h


${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui.cpp
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui_draw.cpp
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui_tables.cpp
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui_widgets.cpp
#Demo file
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/imgui_demo.cpp

#backends
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/backends/imgui_impl_glfw.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/backends/imgui_impl_glfw.cpp
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/backends/imgui_impl_opengl3.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/backends/imgui_impl_opengl3.cpp
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/backends/imgui_impl_opengl3_loader.h

#${CMAKE_CURRENT_LIST_DIR}/misc/debuggers/imgui.natvis
#${CMAKE_CURRENT_LIST_DIR}/misc/debuggers/imgui.natstepfilter

${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/misc/cpp/imgui_stdlib.h
${CMAKE_CURRENT_LIST_DIR}/Include/ImGui/misc/cpp/imgui_stdlib.cpp
)


add_library(imGui ${IMGUI_SOURCE_FILES})
target_include_directories(imGui PUBLIC include/)
