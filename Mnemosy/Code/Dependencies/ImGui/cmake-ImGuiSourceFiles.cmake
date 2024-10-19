# ===== ImGui Source Files ======
# =================================

set(IMGUI_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/ImGui/imconfig.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_internal.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imstb_rectpack.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imstb_textedit.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imstb_truetype.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui.cpp
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_draw.cpp
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_tables.cpp
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_widgets.cpp
#Demo file
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_demo.cpp
#backends
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_impl_glfw.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_impl_opengl3.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_impl_glfw.cpp
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_impl_opengl3.cpp
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_impl_opengl3_loader.h

${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_stdlib.h
${CMAKE_CURRENT_LIST_DIR}/ImGui/imgui_stdlib.cpp
)
