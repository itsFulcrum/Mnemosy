#include "Include/Graphics/Light.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/ModelData.h"
#include "Include/Graphics/ModelLoader.h"

#include "Include/Systems/MeshRegistry.h"

#include <filesystem>
namespace mnemosy::graphics
{
	Light::Light()
	{
		std::filesystem::path meshesPath = MnemosyEngine::GetInstance().GetFileDirectories().GetMeshesPath();
		std::filesystem::path directionalLightMesh = meshesPath / std::filesystem::path("mnemosy_directional_light_mesh.fbx");
		std::filesystem::path pointLightMesh = meshesPath / std::filesystem::path("mnemosy_point_light_mesh.fbx");


		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();

		m_directionalLightMesh_id = meshRegistry.LoadMesh(directionalLightMesh.generic_string());
		m_pointLightMesh_id = meshRegistry.LoadMesh(pointLightMesh.generic_string());


		//ModelLoader modelLoader;
		//m_pDirectionalLightMesh = modelLoader.LoadModelDataFromFile(directionalLightMesh.generic_string().c_str());
		//m_pPointLightMesh = modelLoader.LoadModelDataFromFile(pointLightMesh.generic_string().c_str());

	}
	Light::~Light()
	{
		//delete m_pDirectionalLightMesh;
		//m_pDirectionalLightMesh = nullptr;
		//delete m_pPointLightMesh;
		//m_pPointLightMesh = nullptr;
	}

	const ModelData& Light::GetModelData() {

		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();


		if (m_type == LightType::DIRECTIONAL) {

			return meshRegistry.GetMeshByID(m_directionalLightMesh_id);

			//return *m_pDirectionalLightMesh;
		}
		else if (m_type == LightType::POINT) {
			
			return meshRegistry.GetMeshByID(m_pointLightMesh_id);
			//return *m_pPointLightMesh;

		}

		return meshRegistry.GetMeshByID(m_directionalLightMesh_id);
		//return *m_pDirectionalLightMesh;
	}

	const LightType& Light::GetLightType()
	{
		return m_type;
	}

	int Light::GetLightTypeAsInt()
	{

		return int(m_type);
	}

} // !mnemosy::graphics