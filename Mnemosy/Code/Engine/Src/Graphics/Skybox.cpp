#include "Include/Graphics/Skybox.h"

#include "Include/MnemosyEngine.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Core/FileDirectories.h"


#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/ModelData.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/ModelLoader.h"

namespace mnemosy::graphics
{
	Skybox::Skybox()
	{
		ModelLoader modelLoader;
		m_pModelData = modelLoader.LoadModelDataFromFile("../Resources/Meshes/skyboxMesh.fbx");

		
		m_pCubemap = new Cubemap();
	}

	Skybox::Skybox(const char* imagePath, unsigned int resolution)
	{
		ModelLoader modelLoader;
		m_pModelData = modelLoader.LoadModelDataFromFile("../Resources/Meshes/skyboxMesh.fbx");


		m_pCubemap = new Cubemap();

		// if there is no skybox in the registry generate one 
		bool noEntriesExist = MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().GetVectorOfNames().empty();
		if (noEntriesExist) 
		{
			m_pCubemap->LoadEquirectangularFromFile(imagePath, "SpruitSunrise", resolution, true);
			
			return;
		}

		LoadPreviewSkybox("SpruitSunrise");
	}

	Skybox::~Skybox()
	{

		delete m_pModelData;
		m_pModelData = nullptr;

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

	void Skybox::LoadPreviewSkybox(const std::string& name)
	{
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
	}

	Cubemap& Skybox::GetCubemap()
	{
		return *m_pCubemap;
	}

	ModelData& Skybox::GetModelData()
	{
		return *m_pModelData;
	}

}