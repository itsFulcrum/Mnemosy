#include "Include/Systems/MeshRegistry.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/ModelData.h"




namespace mnemosy::systems {

	MeshRegistry::MeshRegistry() {

	}

	MeshRegistry::~MeshRegistry() {

		// free all the meshes

		if (!m_loadedMeshes.empty()) {

			for (int i = 0; i < m_loadedMeshes.size(); i++) {

				
				// free sub vectors
				for (int m = 0; m < m_loadedMeshes[i]->meshes.size(); m++) {

					m_loadedMeshes[i]->meshes[m].vertecies.clear();
					m_loadedMeshes[i]->meshes[m].indecies.clear();
				}
				m_loadedMeshes[i]->meshes.clear();


				delete m_loadedMeshes[i];
				m_loadedMeshes[i] = nullptr;
			}
		}

		m_loadedMeshes.clear();
		m_loadedMeshesPaths.clear();
	}

	// returns unique ID of the mesh
	uint16_t MeshRegistry::LoadMesh(const std::string& filepath) {

		// check if filepath exists already in the registry and return that mesh id if found

		if (!m_loadedMeshesPaths.empty()) {

			for (int i = 0; i < m_loadedMeshesPaths.size(); i++) {

				// found mesh in the registry
				if (m_loadedMeshesPaths[i] == filepath) {
					//MNEMOSY_DEBUG("MeshRegistry::LoadMesh: found mesh in the registry, {}", filepath)
					return i;
				}
			}
		}


		// mesh is not contained in the registry yet

		graphics::ModelLoader modelLoader;
		graphics::ModelData* model = modelLoader.LoadModelDataFromFile(filepath.c_str());

		//MNEMOSY_DEBUG("MeshRegistry::LoadMesh: Loaded Mesh {}", filepath)

		m_loadedMeshes.push_back(model);
		m_loadedMeshesPaths.push_back(filepath);


		return m_loadedMeshes.size() -1; // yes id is equal to list index
	}

	graphics::ModelData& MeshRegistry::GetMeshByID(uint16_t id) {

		MNEMOSY_ASSERT(id < m_loadedMeshes.size(), "Mesh Id does not exists ");

		return *m_loadedMeshes[id];
	}

	


} // namespace mnemosy::systems