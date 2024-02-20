#include "Engine/Include/Graphics/Skybox.h"


#include "Engine/Include/Graphics/Cubemap.h"
#include "Engine/Include/Graphics/ModelData.h"

#include "Engine/Include/Core/Log.h"
#include "Engine/Include/Graphics/ModelLoader.h"

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
		AssignSkyboxTexture(imagePath,resolution);
	}

	Skybox::~Skybox()
	{

		delete m_pModelData;
		m_pModelData = nullptr;

		delete m_pCubemap;
		m_pCubemap = nullptr;
	}

	void Skybox::AssignSkyboxTexture(const char* imagePath, unsigned int resolution)
	{
		m_pCubemap->LoadEquirectangularFromFile(imagePath, resolution, true);
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