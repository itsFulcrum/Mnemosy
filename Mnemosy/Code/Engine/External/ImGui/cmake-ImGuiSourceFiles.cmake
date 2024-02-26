# ===== ImGui Source Files ======
# =================================

set(IMGUI_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/imconfig.h
${CMAKE_CURRENT_LIST_DIR}/imgui.h
${CMAKE_CURRENT_LIST_DIR}/imgui_internal.h
${CMAKE_CURRENT_LIST_DIR}/imstb_rectpack.h
${CMAKE_CURRENT_LIST_DIR}/imstb_textedit.h
${CMAKE_CURRENT_LIST_DIR}/imstb_truetype.h
${CMAKE_CURRENT_LIST_DIR}/imgui.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui_tables.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp
#Demo file
${CMAKE_CURRENT_LIST_DIR}/imgui_demo.cpp
#backends
${CMAKE_CURRENT_LIST_DIR}/imgui_impl_glfw.h
${CMAKE_CURRENT_LIST_DIR}/imgui_impl_glfw.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui_impl_opengl3.h
${CMAKE_CURRENT_LIST_DIR}/imgui_impl_opengl3.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui_impl_opengl3_loader.h

${CMAKE_CURRENT_LIST_DIR}/imgui_stdlib.h
${CMAKE_CURRENT_LIST_DIR}/imgui_stdlib.cpp
)
