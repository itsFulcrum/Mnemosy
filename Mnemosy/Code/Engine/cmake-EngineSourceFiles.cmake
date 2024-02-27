# ===== ENGINE SOURCE FILES ======
# =================================

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

${ENGINE_SOURCE_PATH}/Include/Core/FileDirectories.h
${ENGINE_SOURCE_PATH}/Src/Core/FileDirectories.cpp


${ENGINE_SOURCE_PATH}/Include/Systems/Input/InputSystem.h
${ENGINE_SOURCE_PATH}/Src/Systems/Input/InputSystem.cpp
${ENGINE_SOURCE_PATH}/Include/Systems/SkyboxAssetRegistry.h
${ENGINE_SOURCE_PATH}/Src/Systems/SkyboxAssetRegistry.cpp

#Utils
${ENGINE_SOURCE_PATH}/Include/Core/Utils/PlatfromUtils_Windows.h
${ENGINE_SOURCE_PATH}/Src/Core/Utils/PlatfromUtils_Windows.cpp

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
