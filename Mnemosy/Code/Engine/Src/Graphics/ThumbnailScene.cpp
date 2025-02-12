#include "Include/Graphics/ThumbnailScene.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Material.h"

#include <filesystem>

namespace mnemosy::graphics
{

	void ThumbnailScene::Init() {
		core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();

		std::filesystem::path cylinderMesh = fd.GetPreviewMeshesPath() / std::filesystem::path("mnemosy_previewMesh_cylinder.fbx");
		//std::filesystem::path standardSkybox = fd.GetTexturesPath() / std::filesystem::path("market.hdr");


		//MNEMOSY_TRACE("Start Init Scene");
		mnemosy::core::Window& window = MnemosyEngine::GetInstance().GetWindow();

		m_camera = std::make_unique<Camera>(window.GetWindowWidth(), window.GetWindowHeight());

		m_mesh = std::make_unique<RenderMesh>(cylinderMesh.generic_string().c_str());

		m_light = std::make_unique<Light>();

		m_skybox = MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().LoadPreviewSkybox(0,false);
		Setup();
	}

	void ThumbnailScene::Shutdown() {

	}

	void ThumbnailScene::Update()
	{
		MnemosyEngine& instance = MnemosyEngine::GetInstance();

		m_camera->SetScreenSize(instance.GetWindow().GetViewportWidth(), instance.GetWindow().GetViewportHeight());

		instance.GetRenderer().SetViewMatrix(m_camera->GetViewMatrix());
		instance.GetRenderer().SetProjectionMatrix(m_camera->GetProjectionMatrix());
	}


	

	void ThumbnailScene::Setup()
	{		
		m_sceneSettings.globalExposure = -0.7f;
		m_sceneSettings.background_color_r = 0.15f;
		m_sceneSettings.background_color_g = 0.15f;
		m_sceneSettings.background_color_b = 0.15f;
		m_sceneSettings.background_rotation = 5.163f;
		m_sceneSettings.background_opacity = 0.0f;
		m_sceneSettings.background_gradientOpacity = 0.0f;
		m_sceneSettings.background_blurRadius = 0.0f;
		//m_sceneSettings.background_blurSteps = 0;

		// camera setup
		m_camera->transform.SetPosition(glm::vec3(0.0f, 0.0f, 1.8f));
		m_camera->transform.SetRotationEulerAngles(glm::vec3(0.0f, 180.0f, 0.0f));

		// base mesh setup
		//m_mesh->LoadMesh("../Resources/Meshes/UnitSphereSmooth.fbx");
		//m_mesh->transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		m_mesh->transform.SetRotationEulerAngles(glm::vec3(0.0f, 0.0f, 0.0f));
		//m_mesh->GetMaterial().Albedo = glm::vec3(0.2f, 0.2f, 0.2f);
		//m_mesh->GetMaterial().Roughness = 0.15f;

		// light setup
		m_light->transform.SetPosition(glm::vec3(-0.9, 0.7f, 1.1f));
		m_light->transform.SetRotationEulerAngles(glm::vec3(0.0f, 0.0f, 0.0));
		m_light->strength = 2.0f;
		//m_light->color = glm::vec3(1.0f, 1.0f, 1.0f);
		m_light->SetType(graphics::LightType::POINT);
		m_light->falloff = 0.24f;


	}

} // !mnemosy::graphics