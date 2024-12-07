#include "Include/Graphics/Scene.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include <json.hpp>

#include "Include/Systems/FolderTreeNode.h"


#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Material.h"

#include <filesystem>

namespace mnemosy::graphics
{

	void Scene::Init() {
		m_currentPreviewMesh = PreviewMesh::Default;	
		m_pbrMaterial = nullptr;


		//MNEMOSY_TRACE("Start Init Scene");
		mnemosy::core::Window& window = MnemosyEngine::GetInstance().GetWindow();


		core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();

		std::filesystem::path previewMesh = fd.GetPreviewMeshesPath() / std::filesystem::path("mnemosy_previewMesh_mnemosy.fbx");

		m_camera = std::make_unique<Camera>(window.GetWindowWidth(), window.GetWindowHeight());
		m_light = std::make_unique<Light>();

		m_mesh = std::make_unique<RenderMesh>(previewMesh.generic_string().c_str());

		//MNEMOSY_TRACE("Scene - light Init");
		std::filesystem::path standardSkybox = fd.GetTexturesPath() / std::filesystem::path("market.hdr");

		m_skybox = new Skybox(standardSkybox.generic_string().c_str(), 1024);
		//MNEMOSY_TRACE("Scene - Skybox Init");

		m_pbrMaterial = new PbrMaterial();

		LoadSceneSettingsFromFile();

		Setup();

	}

	void Scene::Shutdown() {

		SaveSceneSettingsToFile();

		if (m_pbrMaterial) {
			delete m_pbrMaterial;
		}

		if (m_skybox) {
			delete m_skybox;
		}

		if (m_unlitMaterial) {
			delete m_unlitMaterial;
		}

	}


	void Scene::SetPreviewMesh(const PreviewMesh& previewMeshType)
{
		namespace fs = std::filesystem;

		if (previewMeshType == m_currentPreviewMesh)
			return; // already set

		std::filesystem::path meshesPath = MnemosyEngine::GetInstance().GetFileDirectories().GetPreviewMeshesPath();

		if (previewMeshType == PreviewMesh::Custom)
		{
			// makes button appear to load a custom mesh
			m_currentPreviewMesh = PreviewMesh::Custom;
			return;
		}

		std::filesystem::path newMeshPath;

		if (previewMeshType == PreviewMesh::Default)
		{
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_mnemosy.fbx");
		}
		else if (previewMeshType == PreviewMesh::Cube)
		{
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_cube.fbx");
		}
		else if (previewMeshType == PreviewMesh::Plane) {
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_plane.fbx");
		}
		else if (previewMeshType == PreviewMesh::Sphere) {
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_sphere.fbx");
		}
		else if (previewMeshType == PreviewMesh::Cylinder) {
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_cylinder.fbx");
		}
		else if (previewMeshType == PreviewMesh::Suzanne) {
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_suzanne.fbx");
		}
		else if(previewMeshType == PreviewMesh::Fabric){
			newMeshPath = meshesPath / fs::path("mnemosy_previewMesh_fabric.fbx");
		}

		m_mesh->LoadMesh(newMeshPath.generic_string().c_str());
		m_currentPreviewMesh = previewMeshType;

}

	void Scene::SetPbrMaterial(PbrMaterial* pbrMaterial) {
		MNEMOSY_ASSERT(pbrMaterial != nullptr, "pbrMaterial has to be initialized");

		if (m_pbrMaterial) {
			delete m_pbrMaterial;
		}

		m_pbrMaterial = pbrMaterial;
	}

	void Scene::SetUnlitMaterial(UnlitMaterial* unlitMaterial) {
		MNEMOSY_ASSERT(unlitMaterial != nullptr, "Unlit Material has to be initialized");

		if (m_unlitMaterial) {
			delete m_unlitMaterial;
		}

		m_unlitMaterial = unlitMaterial;
	}

	void Scene::SetSkybox(graphics::Skybox* skybox) {
		MNEMOSY_ASSERT(skybox != nullptr, "skybox has to be initialized");

		if (m_skybox) {
			delete m_skybox;
		}

		m_skybox = skybox;
	}

	void Scene::Setup()
	{
		// camera setup
		m_camera->transform.SetPosition(glm::vec3(0.0f, 0.5f, 3.0f));
		m_camera->transform.SetRotationEulerAngles(glm::vec3(0.0f, 180.0f, 0.0f));


		// base mesh setup
		//m_mesh->LoadMesh("../Resources/Meshes/UnitSphereSmooth.fbx");
		//m_mesh->transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		m_mesh->transform.SetRotationEulerAngles(glm::vec3(0.0f, 0.0f, 0.0f));
		//m_mesh->GetMaterial().Albedo = glm::vec3(0.2f, 0.2f, 0.2f);
		//m_mesh->GetMaterial().Roughness = 0.15f;

		// skybox setup
		//MNEMOSY_TRACE("StartGenerateSkyboxes");
		//m_skybox->AssignSkyboxTexture("../Resources/Textures/spruit_sunrise.hdr", 2048);
		//m_skybox->colorTint = glm::vec3(1.0f, 1.0f, 1.0f);
		//m_skybox->exposure = 0.0f;

		// light setup
		m_light->transform.SetPosition(glm::vec3(0.0f, 4.0f, 3.0f));
		m_light->transform.SetRotationEulerAngles(glm::vec3(-45.0f, 0.0f, 0.0));
		m_light->strength = 12.0f;
		//m_light->color = glm::vec3(1.0f, 1.0f, 1.0f);
		m_light->SetType(graphics::LightType::POINT);
		m_light->falloff = 0.5f;


	}

	void Scene::LoadSceneSettingsFromFile()
	{
		namespace fs = std::filesystem;

		fs::path filepath = MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / fs::path("sceneSettings.mnsydata");

		flcrm::JsonSettings file;
		bool success = false;
		file.FileOpen(success,filepath,"Mnemosy Data File", "This file stores user scene settings.");
		if (!success) {
			MNEMOSY_WARN("Failed to load user scene settings file - using defaults: Message: {}", file.ErrorStringLastGet());
		}

		 userSceneSettings.globalExposure	= file.ReadFloat(success, "globalExposure", 0.0f, true);
		 userSceneSettings.background_color_r			= file.ReadFloat(success, "background_color_r", 0.2f, true);
		 userSceneSettings.background_color_g			= file.ReadFloat(success, "background_color_g", 0.2f, true);
		 userSceneSettings.background_color_b			= file.ReadFloat(success, "background_color_b", 0.2f, true);
		 userSceneSettings.background_rotation			= file.ReadFloat(success, "background_rotation", 0.0f, true);
		 userSceneSettings.background_opacity			= file.ReadFloat(success, "background_opacity", 0.0f, true);
		 userSceneSettings.background_gradientOpacity	= file.ReadFloat(success, "background_gradientopacity", 1.0f, true);
		 userSceneSettings.background_blurRadius		= file.ReadFloat(success, "background_blurRadius", 0.0f, true);
		 userSceneSettings.background_blurSteps		= file.ReadInt(success, "background_blurSteps", 0, true);

		 file.FilePrettyPrintSet(true);

		 file.FileClose(success,filepath);
	}

	void Scene::SaveSceneSettingsToFile() {

		namespace fs = std::filesystem;

		fs::path filepath = MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / fs::path("sceneSettings.mnsydata");

		flcrm::JsonSettings file;
		bool success = false;
		file.FileOpen(success, filepath, "Mnemosy Data File", "This file stores user scene settings.");
		if (!success) {
			MNEMOSY_WARN("Failed to load user scene settings file: Message: {}", file.ErrorStringLastGet());
		}

		file.WriteFloat(success, "globalExposure", userSceneSettings.globalExposure);
		file.WriteFloat(success, "background_color_r", userSceneSettings.background_color_r);
		file.WriteFloat(success, "background_color_g", userSceneSettings.background_color_g);
		file.WriteFloat(success, "background_color_b", userSceneSettings.background_color_b);
		file.WriteFloat(success, "background_rotation", userSceneSettings.background_rotation);
		file.WriteFloat(success, "background_opacity", userSceneSettings.background_opacity);
		file.WriteFloat(success, "background_gradientopacity", userSceneSettings.background_gradientOpacity);
		file.WriteFloat(success, "background_blurRadius", userSceneSettings.background_blurRadius);
		file.WriteInt(success,   "background_blurSteps", userSceneSettings.background_blurSteps);

		file.FilePrettyPrintSet(true);

		file.FileClose(success, filepath);
	}

} // !mnemosy::graphics
