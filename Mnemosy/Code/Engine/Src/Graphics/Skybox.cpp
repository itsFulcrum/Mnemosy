#include "Include/Graphics/Skybox.h"

#include "Include/MnemosyEngine.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Core/FileDirectories.h"


#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/ModelData.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Systems/MeshRegistry.h"

#include <filesystem>

namespace mnemosy::graphics
{
	Skybox::Skybox() {

		std::filesystem::path meshesPath = MnemosyEngine::GetInstance().GetFileDirectories().GetMeshesPath();
		std::filesystem::path skyboxMesh = meshesPath / std::filesystem::path("mnemosy_skybox_render_mesh.fbx");// sphere makes for better rendering but for generating skybox from equrectanuglar wee need cubeMesh


		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();
		m_modelData_id = meshRegistry.LoadMesh(skyboxMesh.generic_string());



		//ModelLoader modelLoader;
		//m_pModelData = modelLoader.LoadModelDataFromFile(skyboxMesh.generic_string().c_str()); 
		
		m_pCubemap = new Cubemap();
	}

	Skybox::Skybox(const char* imagePath, unsigned int resolution) {

		std::filesystem::path meshesPath = MnemosyEngine::GetInstance().GetFileDirectories().GetMeshesPath();
		std::filesystem::path skyboxMesh = meshesPath / std::filesystem::path("mnemosy_skybox_render_mesh.fbx");// sphere makes for better rendering but for generating skybox from equrectanuglar wee need cubeMesh


		systems::MeshRegistry& meshRegistry = MnemosyEngine::GetInstance().GetMeshRegistry();
		m_modelData_id = meshRegistry.LoadMesh(skyboxMesh.generic_string());


		m_pCubemap = new Cubemap();

		// if there is no skybox in the registry generate one 
		bool noEntriesExist = MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().GetVectorOfNames().empty();
		if (noEntriesExist) 
		{
			m_pCubemap->LoadEquirectangularFromFile(imagePath, "Market", resolution, true);
			
			return;
		}

		LoadPreviewSkybox("Market");
	}

	Skybox::~Skybox()
	{

		//delete m_pModelData;
		//m_pModelData = nullptr;

		delete m_pCubemap;
		m_pCubemap = nullptr;
	}

	bool Skybox::AssignSkyboxTexture(const char* imagePath, const char* uniqueName, unsigned int resolution, bool savePermanently)
	{
		bool success = false;

		if (savePermanently)
		{
			if( m_pCubemap->LoadEquirectangularFromFile(imagePath, uniqueName,resolution, true) )
				success = true;
		}
		else
		{
			if(m_pCubemap->LoadEquirectangularFromFile(imagePath,"NoNameNeeded", resolution, false))
				success = true;
		}


		return success;
	}

	void Skybox::LoadPreviewSkybox(const std::string& name) {
		
		// TODO load with some kind of identifyer to the filepaths
		MnemosyEngine& engine = MnemosyEngine::GetInstance();

		systems::SkyboxAssetRegistry& registry = engine.GetSkyboxAssetRegistry();


		systems::SkyboxAssetEntry entry = registry.GetEntry(name); // faster than checking before because we go through the vector anyways
		if (entry.skyName == "EntryDoesNotExist")
		{
			MNEMOSY_ERROR("Skybox::LoadPreviewSkybox: Skybox of name: {} - does not exist", name);
			return;
		}
		
		//core::FileDirectories& dirs = MnemosyEngine::GetInstance().GetFileDirectories();
		std::string cubemapsPathString =  engine.GetFileDirectories().GetCubemapsPath().generic_string() + "/";

		std::string colorPath		= cubemapsPathString + entry.colorCubeFile;
		std::string irradiancePath	= cubemapsPathString + entry.irradianceCubeFile;
		std::string prefilterPath	= cubemapsPathString + entry.prefilterCubeFile;
		
		//MNEMOSY_TRACE("LoadSkybox Start");
		m_pCubemap->LoadCubemapsFromKtxFiles(colorPath.c_str(),irradiancePath.c_str(),prefilterPath.c_str());
		//MNEMOSY_TRACE("LoadSkybox End");

		registry.SetNewCurrent(name);
	}

	void Skybox::RemoveCubemap()
	{
		if (m_pCubemap) {
			delete m_pCubemap;
			m_pCubemap = nullptr;
		}
	}

	// scary - cubemap could be null
	Cubemap& Skybox::GetCubemap()
	{
		return *m_pCubemap;
	}

	ModelData& Skybox::GetModelData() {

		return MnemosyEngine::GetInstance().GetMeshRegistry().GetMeshByID(m_modelData_id);
	}

}