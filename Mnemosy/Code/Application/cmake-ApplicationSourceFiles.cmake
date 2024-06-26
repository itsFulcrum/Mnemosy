# ===== APP SOURCE FILES =========
# =================================

set(APP_SOURCE_FILES
# Mnemosy application source files
#include
${APP_SOURCE_PATH}/Include/Application.h
${APP_SOURCE_PATH}/Include/ApplicationConfig.h

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

${APP_SOURCE_PATH}/Include/GuiPanels/MaterialLibraryGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/MaterialLibraryGuiPanel.cpp

${APP_SOURCE_PATH}/Include/GuiPanels/MaterialEditorGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/MaterialEditorGuiPanel.cpp

${APP_SOURCE_PATH}/Include/GuiPanels/DocumentationGuiPanel.h
${APP_SOURCE_PATH}/Src/GuiPanels/DocumentationGuiPanel.cpp

)
