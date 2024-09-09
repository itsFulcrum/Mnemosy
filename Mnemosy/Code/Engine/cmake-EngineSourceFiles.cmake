# ===== ENGINE SOURCE FILES ======
# =================================

set(ENGINE_SOURCE_FILES
# Mnemosy engine source files
${ENGINE_SOURCE_PATH}/Include/MnemosyConfig.h
${ENGINE_SOURCE_PATH}/Include/MnemosyEngine.h
${ENGINE_SOURCE_PATH}/Src/MnemosyEngine.cpp

#Core
${ENGINE_SOURCE_PATH}/Include/Core/Window.h
${ENGINE_SOURCE_PATH}/Src/Core/Window.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Logger.h
${ENGINE_SOURCE_PATH}/Src/Core/Logger.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Log.h
${ENGINE_SOURCE_PATH}/Include/Core/Clock.h
${ENGINE_SOURCE_PATH}/Src/Core/Clock.cpp
${ENGINE_SOURCE_PATH}/Include/Core/FileDirectories.h
${ENGINE_SOURCE_PATH}/Src/Core/FileDirectories.cpp



#Systems
${ENGINE_SOURCE_PATH}/Include/Systems/Input/InputSystem.h
${ENGINE_SOURCE_PATH}/Src/Systems/Input/InputSystem.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/SkyboxAssetRegistry.h
${ENGINE_SOURCE_PATH}/Src/Systems/SkyboxAssetRegistry.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/MaterialLibraryRegistry.h
${ENGINE_SOURCE_PATH}/Src/Systems/MaterialLibraryRegistry.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/FolderTreeNode.h
${ENGINE_SOURCE_PATH}/Src/Systems/FolderTreeNode.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/FolderTree.h
${ENGINE_SOURCE_PATH}/Src/Systems/FolderTree.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/ThumbnailManager.h
${ENGINE_SOURCE_PATH}/Src/Systems/ThumbnailManager.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/TextureGenerationManager.h
${ENGINE_SOURCE_PATH}/Src/Systems/TextureGenerationManager.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/ExportManager.h
${ENGINE_SOURCE_PATH}/Src/Systems/ExportManager.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/JsonKeys.h
${ENGINE_SOURCE_PATH}/Include/Systems/UserSettingsManager.h
${ENGINE_SOURCE_PATH}/Src/Systems/UserSettingsManager.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/MeshRegistry.h
${ENGINE_SOURCE_PATH}/Src/Systems/MeshRegistry.cpp

#Utils
${ENGINE_SOURCE_PATH}/Include/Core/Utils/PlatfromUtils_Windows.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/PlatfromUtils_Windows.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Utils/DropManager_Windows.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/DropManager_Windows.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Utils/DropHandler_Windows.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/DropHandler_Windows.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Utils/FileWatcher.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/FileWatcher.cpp
${ENGINE_SOURCE_PATH}/Include/Core/Utils/StringUtils.h

# Graphics
${ENGINE_SOURCE_PATH}/Include/Graphics/Utils/ShaderIncludeParser.h

${ENGINE_SOURCE_PATH}/Include/Graphics/Utils/KtxImage.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Utils/KtxImage.cpp

${ENGINE_SOURCE_PATH}/Include/Graphics/Shader.h
${ENGINE_SOURCE_PATH}/Src/Graphics/Shader.cpp

${ENGINE_SOURCE_PATH}/include/Graphics/MeshData.h
${ENGINE_SOURCE_PATH}/include/Graphics/ModelData.h
${ENGINE_SOURCE_PATH}/include/Graphics/ModelLoader.h
${ENGINE_SOURCE_PATH}/Src/Graphics/ModelLoader.cpp
${ENGINE_SOURCE_PATH}/Include/Graphics/TextureDefinitions.h
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
${ENGINE_SOURCE_PATH}/Include/Graphics/ThumbnailScene.h
${ENGINE_SOURCE_PATH}/Src/Graphics/ThumbnailScene.cpp

#user interface
${ENGINE_SOURCE_PATH}/Include/Gui/UserInterface.h
${ENGINE_SOURCE_PATH}/Src/Gui/UserInterface.cpp
${ENGINE_SOURCE_PATH}/Include/Gui/GuiPanel.h
${ENGINE_SOURCE_PATH}/Src/Gui/GuiPanel.cpp

)
