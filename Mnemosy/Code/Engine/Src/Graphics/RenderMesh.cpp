#include "Include/Graphics/RenderMesh.h"


#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Systems/MeshRegistry.h"


#include <string>
//#include "Include/Graphics/ModelLoader.h"
//#include "Include/Graphics/Material.h"
//#include "Include/Graphics/ModelData.h"

namespace mnemosy::graphics
{
	RenderMesh::RenderMesh() {

		LoadMesh("../Resources/Meshes/UnitCube.fbx");
	}

	RenderMesh::RenderMesh(const char* path) {

		LoadMesh(path);
	}

	RenderMesh::~RenderMesh() {
		
		//if (m_modelData) {
		//	delete m_modelData;
		//	m_modelData = nullptr;
		//}
	}

	void RenderMesh::LoadMesh(const char* path) {

		//if (m_modelData) {
		//
		//	ClearModelData();
		//}


		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();

		std::string filepathString = std::string(path);
		m_modelData_id = meshRegistry.LoadMesh(filepathString);


		//ModelLoader modelLoader;
		//m_modelData = modelLoader.LoadModelDataFromFile(path);
	}




	ModelData& RenderMesh::GetModelData() {

		return MnemosyEngine::GetInstance().GetMeshRegistry().GetMeshByID(m_modelData_id);
		//MNEMOSY_ASSERT(m_modelData, "RenderMesh has no model data yet");
		///return *m_modelData;
	}

	void RenderMesh::ClearModelData() {

		//for (int i = 0; i < m_modelData->meshes.size(); i++) {
		//
		//	m_modelData->meshes[i].vertecies.clear();
		//	m_modelData->meshes[i].indecies.clear();
		//}
		//m_modelData->meshes.clear();
		//
		//delete m_modelData;
		//m_modelData = nullptr;
	}

} // !mnemosy::graphics