#include "Include/Core/FileDirectories.h"

#include "Include/Core/Log.h"




namespace mnemosy::core
{
	FileDirectories::FileDirectories()
	{
		m_mnemosyInternalResourcesDirectory = fs::directory_entry(R"(../Resources)"); /// std::filesystem::path("../Resources");
	}
	FileDirectories::~FileDirectories()
	{

	}

	fs::path FileDirectories::GetResourcesPath()
	{

		if (m_mnemosyInternalResourcesDirectory.exists()) 
		{
			return m_mnemosyInternalResourcesDirectory.path();
		}
		else
		{
			MNEMOSY_ERROR("Resources Path does not exist.. Did you delete it?: {} ", m_mnemosyInternalResourcesDirectory.path().generic_string());
		}

		return std::filesystem::path("");
	}
	fs::path FileDirectories::GetMeshesPath()
	{
		fs::path meshesPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Meshes");
		return meshesPath;
	}
	fs::path FileDirectories::GetTexturesPath()
	{
		fs::path texturesPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Textures");
		return texturesPath;
	}
	fs::path FileDirectories::GetDataPath()
	{
		fs::path dataPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Data");
		return dataPath;
	}
	fs::path FileDirectories::GetCubemapsPath()
	{
		fs::path cubemapsPath = GetTexturesPath() / fs::path("Cubemaps");
		return cubemapsPath;
	}
	fs::path FileDirectories::GetShadersPath()
	{

		fs::path shadersPath = m_mnemosyInternalResourcesDirectory.path() / fs::path("Shaders");
		return shadersPath;
	}
} // !mnemosy::core