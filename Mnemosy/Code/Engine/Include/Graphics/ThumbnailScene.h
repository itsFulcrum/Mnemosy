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

	class ThumbnailScene
	{
	public:
		ThumbnailScene();
		~ThumbnailScene();
		
		void Update();

		// getters
		Camera& GetCamera()	{ return *m_camera; }
		RenderMesh& GetMesh() { return *m_mesh; }
		Light& GetLight() {	return *m_light;}
		Skybox& GetSkybox() { return *m_skybox; }

	private:
		void Setup();

		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<RenderMesh> m_mesh;
		std::unique_ptr<Light> m_light;
		std::unique_ptr<Skybox> m_skybox;
	};

} // mnemosy::graphics

#endif // !THUMBNAIL_SCENE_H
