#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H
// std librarys
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <view/MeshData.h>
#include <view/ModelData.h>


/*
	with this currentimplementation of a model it only allows to have one material
	so even if submeshes have different materials assigned to them this class will ignore it
*/

class ModelLoader
{
public:
	ModelLoader() {}

	ModelData LoadModelDataFromFile(std::string const& path)
	{
		return	M_LoadModel(path);
	}

private:
	std::string m_fileDirectory;

	ModelData M_LoadModel(std::string const& path)
	{
		ModelData modelData;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		// could use several options such as aiProccess_GenNormals, aiProcess_OptimizeMeshes,aiProcess_SplitLargeMeshes,aiProcess_FlipUVs
		// importer.ReadFile(path, aiProcess_Triangulate | aiProccess_GenNormals );

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			// build linking erro when using importer.getErroString()
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return modelData;
		}

		m_fileDirectory = path.substr(0, path.find_first_of('/'));
		M_ProcessNode(scene->mRootNode, scene, modelData);

		return modelData;

	}
	void M_ProcessNode(aiNode* node, const aiScene* scene, ModelData& modelData)
	{

		// process all the node's meshes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			//aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			// !! Somthing broken with the import her.
			// semes to work for at least single mesh files..
			aiMesh* mesh = scene->mMeshes[i];
			modelData.meshes.push_back(M_ProcessMesh(mesh, scene));
			
		}
		// recursively call for all the childrens 
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			M_ProcessNode(node->mChildren[i], scene, modelData);
		}
	}
	MeshData M_ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			// process data
			glm::vec3 position;
			position.x = mesh->mVertices[i].x;
			position.y = mesh->mVertices[i].y;
			position.z = mesh->mVertices[i].z;
			vertex.position = position;



			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
			vertex.normal = normal;

			glm::vec3 tangent;
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;
			vertex.tangent = tangent;


			glm::vec3 bitangent;
			bitangent.x = mesh->mBitangents[i].x;
			bitangent.y = mesh->mBitangents[i].y;
			bitangent.z = mesh->mBitangents[i].z;
			vertex.bitangent = bitangent;


			if (mesh->mColors[0])
			{
				glm::vec3 color;
				color.r = mesh->mColors[0][i].r;
				color.g = mesh->mColors[0][i].g;
				color.b = mesh->mColors[0][i].b;
				vertex.color = color;
			}
			else
			{
				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // default white
			}
			// checking if texCoords Exist
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 uv0;
				uv0.x = mesh->mTextureCoords[0][i].x;
				uv0.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = uv0;
			}
			else
			{
				vertex.texCoords = glm::vec2(0.5f, 0.5f); // all at 0
			}


			vertices.push_back(vertex);
		}
		// process indices

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}


		return MeshData(vertices, indices);
	}

};


#endif // !MODEL_LOADER_H
