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
	RenderMesh::RenderMesh(const char* path) {

		LoadMesh(path);
	}

	RenderMesh::~RenderMesh() {}

	void RenderMesh::LoadMesh(const char* path) {
		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();

		std::string filepathString = std::string(path);
		m_modelData_id = meshRegistry.LoadMesh(filepathString);
	}

	ModelData& RenderMesh::GetModelData() {

		return MnemosyEngine::GetInstance().GetMeshRegistry().GetMeshByID(m_modelData_id);
	}

} // !mnemosy::graphics