#ifndef MESH_REGISTRY_H
#define MESH_REGISTRY_H

#include <string>
#include <vector>

namespace mnemosy::graphics {
	struct ModelData;
}


namespace mnemosy::graphics {

	struct ScreenQuad {

	public:


	private:


	};
}

// This mesh registry works because the loaded meshes never get freed until the program ends,
//  -> so we can use the position in the vector as id and be super fast

namespace mnemosy::systems {

	class MeshRegistry {

	public:
		MeshRegistry()  = default;
		~MeshRegistry() = default;

		void Init();
		void Shutdown();

		uint16_t LoadMesh(const std::string& filepath);
		graphics::ModelData& GetMeshByID(uint16_t id);


		unsigned int& GetScreenQuadVAO();
		graphics::ModelData& GetSkyboxRenderMesh();

	private:

		// these two are always in sync so a mesh m_loadedMeshes[0] was loaded from file system path m_loadedMeshesPaths[0]
		std::vector<graphics::ModelData*> m_loadedMeshes;
		std::vector<std::string> m_loadedMeshesPaths;

		uint16_t m_skybox_render_ModelData_ID;

		unsigned int m_ScreenQuad_VBO = 0;
		unsigned int m_ScreenQuad_VAO = 0;

		const float m_screenQuadVertices[24] = {
			// triangle 1
			// positions xy		uvs
			-1.0f, -1.0f,		0.0f,0.0f,
			 1.0f,  1.0f,		1.0f,1.0f,
			-1.0f,  1.0f,		0.0f,1.0f,
			// triangle 2
			-1.0f, -1.0f,		0.0f,0.0f,
			 1.0f, -1.0f,		1.0f,0.0f,
			 1.0f,  1.0f,		1.0f,1.0f
		};


	};


} // namespace mnemosy::systems


#endif // !MESH_REGISTRY_H
