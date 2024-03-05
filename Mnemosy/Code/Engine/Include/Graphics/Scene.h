#ifndef SCENE_H
#define SCENE_H

#include <memory>

namespace mnemosy::graphics
{
	class Camera;
	class RenderMesh;
	class Skybox;
	class Light;
}

namespace mnemosy::graphics
{
	enum PreviewMesh
	{
		Custom = 0,
		Default = 1,
		Cube = 2,
		Plane = 3,
		Sphere = 4,
		Cylinder = 5,
		Suzanne = 6
	};

	class Scene
	{
	public:
		Scene();
		~Scene();
		
		void Update();

		// getters
		Camera& GetCamera()	{ return *m_camera; }
		RenderMesh& GetMesh() { return *m_mesh; }
		RenderMesh& GetGizmoMesh() { return *m_gizmoMesh; }
		Light& GetLight() {	return *m_light;}
		Skybox& GetSkybox() { return *m_skybox; }
		const PreviewMesh& GetCurrentPreviewMesh() { return m_currentPreviewMesh; }

		void SetPreviewMesh(const PreviewMesh& previewMeshType);

	private:
		void Setup();

		PreviewMesh m_currentPreviewMesh = PreviewMesh::Default;

		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<RenderMesh> m_mesh;
		std::unique_ptr<RenderMesh> m_gizmoMesh;
		std::unique_ptr<Light> m_light;
		std::unique_ptr<Skybox> m_skybox;
	};

} // mnemosy::graphics

#endif // !SCENE_H
