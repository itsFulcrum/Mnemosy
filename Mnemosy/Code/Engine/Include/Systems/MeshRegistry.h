#ifndef MESH_REGISTRY_H
#define MESH_REGISTRY_H

#include <string>
#include <vector>

namespace mnemosy::graphics {
	struct ModelData;
}


// This mesh registry works because the loaded meshes never get freed until the program ends,
//  -> so we can use the position in the vector as id and be super fast

namespace mnemosy::systems {

	class MeshRegistry {

	public:
		MeshRegistry();
		~MeshRegistry();


		uint16_t LoadMesh(const std::string& filepath);
		graphics::ModelData& GetMeshByID(uint16_t id);

	private:
		std::vector<graphics::ModelData*> m_loadedMeshes;
		std::vector<std::string> m_loadedMeshesPaths;
	};


} // namespace mnemosy::systems


#endif // !MESH_REGISTRY_H
