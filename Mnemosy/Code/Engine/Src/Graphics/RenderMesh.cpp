#include "Include/Graphics/RenderMesh.h"

#include "Include/Core/Log.h"

#include "Include/Graphics/ModelLoader.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/ModelData.h"

namespace mnemosy::graphics
{
	RenderMesh::RenderMesh()
	{
		m_material = new Material();

		LoadMesh("../Resources/Meshes/UnitCube.fbx");
	}

	RenderMesh::RenderMesh(const char* path)
	{
		m_material = new Material();

		LoadMesh(path);
	}

	RenderMesh::~RenderMesh()
	{

		if (m_material)
		{
			delete m_material;
			m_material = nullptr;
		}


		if (m_modelData)
		{
			delete m_modelData;
			m_modelData = nullptr;
		}

	}

	void RenderMesh::SetMaterial(Material* const material)
	{
		if (m_material)
		{
			delete m_material;
			m_material = nullptr;
		}

		m_material = material;

	}

	void RenderMesh::LoadMesh(const char* path)
	{
		if (m_modelData)
		{
			ClearModelData();
		}

		ModelLoader modelLoader;
		m_modelData = modelLoader.LoadModelDataFromFile(path);
	}




	ModelData& RenderMesh::GetModelData()
	{
		MNEMOSY_ASSERT(m_modelData, "RenderMesh has no model data yet");

		return *m_modelData;
	}

	void RenderMesh::ClearModelData()
	{
		for (int i = 0; i < m_modelData->meshes.size(); i++)
		{
			m_modelData->meshes[i].vertecies.clear();
			m_modelData->meshes[i].indecies.clear();
		}
		m_modelData->meshes.clear();

		delete m_modelData;
		m_modelData = nullptr;

	}

} // !mnemosy::graphics