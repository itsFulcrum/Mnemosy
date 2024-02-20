#include "Engine/Include/Graphics/RenderMesh.h"

#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Graphics/ModelLoader.h"

#include "Engine/Include/Graphics/Material.h"
#include "Engine/Include/Graphics/ModelData.h"



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

	void RenderMesh::SetMaterial(Material* material)
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
			delete m_modelData;
			m_modelData = nullptr;
		}

		ModelLoader modelLoader;
		m_modelData = modelLoader.LoadModelDataFromFile(path);
	}

	ModelData& RenderMesh::GetModelData()
	{
		MNEMOSY_ASSERT(m_modelData, "RenderMesh has no model data yet");

		return *m_modelData;
	}

}