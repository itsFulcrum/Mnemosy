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


	private:
		void Setup();

		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<RenderMesh> m_mesh;
		std::unique_ptr<RenderMesh> m_gizmoMesh;
		std::unique_ptr<Light> m_light;
		std::unique_ptr<Skybox> m_skybox;
	};


}

#endif // !SCENE_H
