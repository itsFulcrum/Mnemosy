#ifndef SCENE_H
#define SCENE_H

#include "Include/Graphics/SceneSettings.h"
#include <memory>

namespace mnemosy::systems
{
	enum LibEntryType;
	struct LibEntry;
}

namespace mnemosy::graphics
{
	class Camera;
	class RenderMesh;
	class PbrMaterial;
	class UnlitMaterial;
	class Skybox;
	class Light;
}

namespace mnemosy::graphics
{
	enum PreviewMesh
	{
		Custom		= 0,
		Default		= 1,
		Cube		= 2,
		Plane		= 3,
		Sphere		= 4,
		Cylinder	= 5,
		Suzanne		= 6,
		Fabric 		= 7,
		Count		= 8,
	};

	class Scene
	{
	public:
		Scene()  = default;
		~Scene() = default;

		void Init();
		void Shutdown();

		// getters
		Camera& GetCamera()	{ return *m_camera; }
		RenderMesh& GetMesh() { return *m_mesh; }

		Light& GetLight() {	return *m_light;}
		const PreviewMesh& GetCurrentPreviewMesh() { return m_currentPreviewMesh; }
		void SetPreviewMesh(const PreviewMesh& previewMeshType);


		PbrMaterial& GetPbrMaterial() { return *m_pbrMaterial; }
		void SetPbrMaterial(PbrMaterial* material);


		UnlitMaterial* GetUnlitMaterial() { return m_unlitMaterial; }
		void SetUnlitMaterial(UnlitMaterial* unlitMaterial);

		Skybox& GetSkybox() { return *m_skybox; }
		void SetSkybox(graphics::Skybox* skybox);

		SceneSettings userSceneSettings;

	private:
		void Setup();
		void LoadSceneSettingsFromFile();
		void SaveSceneSettingsToFile();

		PreviewMesh m_currentPreviewMesh = PreviewMesh::Default;

		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<RenderMesh> m_mesh;
		std::unique_ptr<Light> m_light;

		PbrMaterial* m_pbrMaterial = nullptr;
		UnlitMaterial* m_unlitMaterial = nullptr;
		Skybox* m_skybox = nullptr;

	};

} // mnemosy::graphics

#endif // !SCENE_H
