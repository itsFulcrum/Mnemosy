#ifndef GRAPHICS_MODEL_LOADER_H
#define GRAPHICS_MODEL_LOADER_H

#include "Include/Graphics/ModelData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

/*
	with this currentimplementation of a model it only allows to have one material
	so even if submeshes have different materials assigned to them this class will ignore it
*/
namespace mnemosy::graphics
{

	class ModelLoader
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		ModelData* LoadModelDataFromFile(std::string const& path);

	private:
		std::string m_fileDirectory;

		ModelData* M_LoadModel(std::string const& path);

		void M_ProcessNode(aiNode* node, const aiScene* scene, ModelData& modelData);

		MeshData M_ProcessMesh(aiMesh* mesh, const aiScene* scene);

		void M_SetupMesh(MeshData& _meshData, std::vector<VertexData> Vertecies, std::vector<unsigned int> Indecies);

	};

}	// mnemosy::graphics



#endif // !GRAPHICS_MODEL_LOADER_H
