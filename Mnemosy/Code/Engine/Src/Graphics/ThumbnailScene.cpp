#include "Include/Graphics/ThumbnailScene.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Material.h"

#include <filesystem>

namespace mnemosy::graphics
{
	ThumbnailScene::ThumbnailScene() {


		core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();

		std::filesystem::path cylinderMesh = fd.GetPreviewMeshesPath() / std::filesystem::path("mnemosy_previewMesh_cylinder.fbx");
		std::filesystem::path standardSkybox = fd.GetTexturesPath() / std::filesystem::path("brown_photostudio.hdr");


		//MNEMOSY_TRACE("Start Init Scene");
		mnemosy::core::Window& window = MnemosyEngine::GetInstance().GetWindow();
		
		m_camera = std::make_unique<Camera>(window.GetWindowWidth(), window.GetWindowHeight());
		
		m_mesh = std::make_unique<RenderMesh>(cylinderMesh.generic_string().c_str());
	
		m_light = std::make_unique<Light>();
		
		m_skybox = std::make_unique<Skybox>(standardSkybox.generic_string().c_str(), 1024);


		Setup();
	}
	ThumbnailScene::~ThumbnailScene() {

	}

	void ThumbnailScene::Update()
	{
		MnemosyEngine& instance = MnemosyEngine::GetInstance();


		m_camera->SetScreenSize(instance.GetWindow().GetViewportWidth(), instance.GetWindow().GetViewportHeight());
		//m_scene->GetCamera().SetScreenSize(m_pWindow->GetWindowWidth(), m_pWindow->GetWindowHeight());

		instance.GetRenderer().SetViewMatrix(m_camera->GetViewMatrix());
		instance.GetRenderer().SetProjectionMatrix(m_camera->GetProjectionMatrix());

		//m_pRenderer->SetViewMatrix(m_scene->GetCamera().GetViewMatrix());
		//m_pRenderer->SetProjectionMatrix(m_scene->GetCamera().GetProjectionMatrix());
	}



	/*
	void ThumbnailScene::SetPreviewMesh(const PreviewMesh& previewMeshType) {
		if (previewMeshType == m_currentPreviewMesh)
			return; // already set
		

		if (previewMeshType == PreviewMesh::Custom)
		{
			// makes button appear to load a custom mesh
			m_currentPreviewMesh = PreviewMesh::Custom;
			return;
		}
		if (previewMeshType == PreviewMesh::Default)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_mnemosy.fbx");
			m_currentPreviewMesh = PreviewMesh::Default;
			return;
		}
		if (previewMeshType == PreviewMesh::Cube)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_cube.fbx");
			m_currentPreviewMesh = PreviewMesh::Cube;
			return;
		}
		if (previewMeshType == PreviewMesh::Plane)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_plane.fbx");
			m_currentPreviewMesh = PreviewMesh::Plane;
			return;
		}
		if (previewMeshType == PreviewMesh::Sphere)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_sphere.fbx");
			m_currentPreviewMesh = PreviewMesh::Sphere;
			return;
		}
		if (previewMeshType == PreviewMesh::Cylinder)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_cylinder.fbx");
			m_currentPreviewMesh = PreviewMesh::Cylinder;
			return;
		}
		if (previewMeshType == PreviewMesh::Suzanne)
		{
			m_mesh->LoadMesh("../Resources/Meshes/PreviewMeshes/mnemosy_previewMesh_suzanne.fbx");
			m_currentPreviewMesh = PreviewMesh::Suzanne;
			return;
		}
	}
	*/

	

	void ThumbnailScene::Setup()
	{
		// camera setup
		m_camera->transform.SetPosition(glm::vec3(0.0f, 0.0f, 1.8f));
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
		m_skybox->rotation = 2;
		m_skybox->colorTint = glm::vec3(0.3f, 0.3f, 0.3f);
		m_skybox->exposure = -0.5f;
		m_skybox->blurSteps = 0;
		m_skybox->backgroundColor = glm::vec3(0.2f, 0.2f, 0.2f);
		m_skybox->opacity = 0.0f;
		m_skybox->gradientOpacity = 1.0f;



		// light setup
		m_light->transform.SetPosition(glm::vec3(0.0f, 4.0f, 3.0f));
		m_light->transform.SetRotationEulerAngles(glm::vec3(-45.0f, 0.0f, 0.0));
		m_light->strength = 50.0f;
		//m_light->color = glm::vec3(1.0f, 1.0f, 1.0f);
		m_light->SetType(graphics::LightType::POINT);
		m_light->falloff = 0.5f;


	}

} // !mnemosy::graphics