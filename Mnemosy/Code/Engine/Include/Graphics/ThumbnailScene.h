#ifndef THUMBNAIL_SCENE_H
#define THUMBNAIL_SCENE_H

#include <memory>

namespace mnemosy::graphics
{
	class Camera;
	class RenderMesh;
	class Material;
	class Skybox;
	class Light;
}

namespace mnemosy::graphics
{
	/*enum PreviewMesh
	{
		Custom = 0,
		Default = 1,
		Cube = 2,
		Plane = 3,
		Sphere = 4,
		Cylinder = 5,
		Suzanne = 6
	};*/

	class ThumbnailScene
	{
	public:
		ThumbnailScene();
		~ThumbnailScene();
		
		void Update();

		// getters
		Camera& GetCamera()	{ return *m_camera; }
		RenderMesh& GetMesh() { return *m_mesh; }
		//RenderMesh& GetGizmoMesh() { return *m_gizmoMesh; }
		Light& GetLight() {	return *m_light;}
		Skybox& GetSkybox() { return *m_skybox; }
		//Material& GetActiveMaterial() { return *m_activeMaterial; }
		//const PreviewMesh& GetCurrentPreviewMesh() { return m_currentPreviewMesh; }

		//void SetPreviewMesh(const PreviewMesh& previewMeshType);
		//void SetMaterial(Material* material);

	private:
		void Setup();

		//PreviewMesh m_currentPreviewMesh = PreviewMesh::Default;

		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<RenderMesh> m_mesh;
		//std::unique_ptr<RenderMesh> m_gizmoMesh;
		std::unique_ptr<Light> m_light;
		std::unique_ptr<Skybox> m_skybox;



		//Material* m_activeMaterial = nullptr;
	};

} // mnemosy::graphics

#endif // !THUMBNAIL_SCENE_H
