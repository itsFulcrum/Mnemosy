#include "Include/Graphics/Light.h"

#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/ModelLoader.h"

namespace mnemosy::graphics
{
	Light::Light()
	{
		ModelLoader modelLoader;
		m_pDirectionalLightMesh = modelLoader.LoadModelDataFromFile("../Resources/Meshes/Forward-z.fbx");
		m_pPointLightMesh = modelLoader.LoadModelDataFromFile("../Resources/Meshes/pointLightMesh.fbx");

	}
	Light::~Light()
	{
		delete m_pDirectionalLightMesh;
		m_pDirectionalLightMesh = nullptr;
		delete m_pPointLightMesh;
		m_pPointLightMesh = nullptr;
	}

	ModelData& Light::GetModelData()
	{
		if (m_type == LightType::DIRECTIONAL)
		{
			return *m_pDirectionalLightMesh;
		}
		else if (m_type == LightType::POINT)
		{
			return *m_pPointLightMesh;
		}

		return *m_pDirectionalLightMesh;
	}

	LightType Light::GetLightType()
	{
		return m_type;
	}

	int Light::GetLightTypeAsInt()
	{

		return int(m_type);
	}

} // !mnemosy::graphics