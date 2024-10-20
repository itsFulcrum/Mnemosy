#include "Include/Graphics/Scene.h"

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
	Scene::Scene()
	{
		//MNEMOSY_TRACE("Start Init Scene");
		mnemosy::core::Window& window = MnemosyEngine::GetInstance().GetWindow();


		core::FileDirectories& fd =  MnemosyEngine::GetInstance().GetFileDirectories();

		std::filesystem::path previewMesh = fd.GetPreviewMeshesPath() / std::filesystem::path("mnemosy_previewMesh_mnemosy.fbx");


		m_camera = std::make_unique<Camera>(window.GetWindowWidth(), window.GetWindowHeight());
		m_light = std::make_unique<Light>();




		m_mesh = std::make_unique<RenderMesh>(previewMesh.generic_string().c_str());

#ifdef MNEMOSY_RENDER_GIZMO
		std::filesystem::path gizmoMesh = fd.GetMeshesPath() / std::filesystem::path("mnemosy_gizmo_mesh.fbx");
		m_gizmoMesh = std::make_unique<RenderMesh>(gizmoMesh.generic_string().c_str());
#endif // MNEMOSY_RENDER_GIZMO

		//MNEMOSY_TRACE("Scene - light Init");
		std::filesystem::path standardSkybox = fd.GetTexturesPath() / std::filesystem::path("market.hdr");
		m_skybox = std::make_unique<Skybox>(standardSkybox.generic_string().c_str(), 1024);
		//MNEMOSY_TRACE("Scene - Skybox Init");

		m_activeMaterial = new Material();

		Setup();
	}
	Scene::~Scene() {

		if (m_activeMaterial) {

			delete m_activeMaterial;
			m_activeMaterial = nullptr;
		}
	}

	void Scene::Update()
	{
		MnemosyEngine& instance = MnemosyEngine::GetInstance();


		m_camera->SetScreenSize(instance.GetWindow().GetViewportWidth(), instance.GetWindow().GetViewportHeight());
		//m_scene->GetCamera().SetScreenSize(m_pWindow->GetWindowWidth(), m_pWindow->GetWindowHeight());

		instance.GetRenderer().SetViewMatrix(m_camera->GetViewMatrix());
		instance.GetRenderer().SetProjectionMatrix(m_camera->GetProjectionMatrix());

		//m_pRenderer->SetViewMatrix(m_scene->GetCamera().GetViewMatrix());
		//m_pRenderer->SetProjectionMatrix(m_scene->GetCamera().GetProjectionMatrix());
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

	void Scene::SetMaterial(Material* material) {
		if (m_activeMaterial) {
			delete m_activeMaterial;
			m_activeMaterial = nullptr;
		}

		m_activeMaterial = material;

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


#ifdef MNEMOSY_RENDER_GIZMO
		// gizmo mesh setup
		// gizmo Mesh could just be a modelData object directly. it doesnt need a material instance but it die
		//m_gizmoMesh->transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		m_gizmoMesh->transform.SetRotationEulerAngles(glm::vec3(0.0f, 0.0f, 0.0f));
		m_gizmoMesh->transform.SetScale(glm::vec3(0.03f, 0.03f, 0.03f));
#endif // MNEMOSY_RENDER_GIZMO


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

} // !mnemosy::graphics
