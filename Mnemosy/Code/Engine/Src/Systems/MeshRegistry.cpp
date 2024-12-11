#include "Include/Systems/MeshRegistry.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/Clock.h"
#include "Include/Core/Log.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/ModelData.h"

#include <glad/glad.h>

#include <filesystem>

namespace mnemosy::systems {

	void MeshRegistry::Init() {

		// Load Skybox Render Mesh

		std::filesystem::path skyboxMeshPath = MnemosyEngine::GetInstance().GetFileDirectories().GetMeshesPath() / std::filesystem::path("mnemosy_skybox_render_mesh.fbx");

		m_skybox_render_ModelData_ID = LoadMesh(skyboxMeshPath.generic_string());
		
		// Init screen quad Mesh buffers
		
		m_ScreenQuad_VBO = 0;
		m_ScreenQuad_VAO = 0;
		
		if (m_ScreenQuad_VBO == 0) {
			glGenBuffers(1, &m_ScreenQuad_VBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuad_VBO);
		}

		if (m_ScreenQuad_VAO == 0) {
			glGenVertexArrays(1, &m_ScreenQuad_VAO);
			glBindVertexArray(m_ScreenQuad_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuad_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(m_screenQuadVertices), m_screenQuadVertices, GL_STATIC_DRAW);

			// Setup Attributes
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		}

		MNEMOSY_TRACE("VAO_ ID: {}", m_ScreenQuad_VAO);

	}

	void MeshRegistry::Shutdown()
	{
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

		// delete screen quad
		if (m_ScreenQuad_VBO != 0) {
			glDeleteBuffers(1, &m_ScreenQuad_VBO);
			m_ScreenQuad_VBO = 0;
		}

		if (m_ScreenQuad_VAO != 0) {
			glDeleteVertexArrays(1, &m_ScreenQuad_VAO);
			m_ScreenQuad_VAO = 0;
		}
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

		double beginTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		// mesh is not contained in the registry yet

		graphics::ModelLoader modelLoader;
		graphics::ModelData* model = modelLoader.LoadModelDataFromFile(filepath.c_str());

		m_loadedMeshes.push_back(model);
		m_loadedMeshesPaths.push_back(filepath);


		double endTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		MNEMOSY_TRACE("Loaded New Mesh in {}, seconds", endTime - beginTime);



		return m_loadedMeshes.size() -1; // yes id is equal to list index
	}

	graphics::ModelData& MeshRegistry::GetMeshByID(uint16_t id) {

		MNEMOSY_ASSERT(id < m_loadedMeshes.size(), "Mesh Id does not exists ");

		return *m_loadedMeshes[id];
	}



	unsigned int& MeshRegistry::GetScreenQuadVAO() {

		return m_ScreenQuad_VAO;
	}

	graphics::ModelData& MeshRegistry::GetSkyboxRenderMesh() {
		return GetMeshByID(m_skybox_render_ModelData_ID);
	}

	


} // namespace mnemosy::systems