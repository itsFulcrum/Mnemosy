#include "Engine/Include/Graphics/ModelLoader.h"

#include "Engine/Include/Core/Log.h"

#include <glad/glad.h>

#include <memory>
#include <vector>

namespace mnemosy::graphics
{
	// public
	ModelData* ModelLoader::LoadModelDataFromFile(std::string const& path)
	{
		return	M_LoadModel(path);
	}

	// private

	ModelData* ModelLoader::M_LoadModel(std::string const& path)
	{
		ModelData* modelData = new ModelData();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		// could use several options such as aiProccess_GenNormals, aiProcess_OptimizeMeshes,aiProcess_SplitLargeMeshes,aiProcess_FlipUVs
		// importer.ReadFile(path, aiProcess_Triangulate | aiProccess_GenNormals );

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			// build linking erro when using importer.getErroString()
			
			MNEMOSY_ERROR("Faild to load Model from file: {} \n Filepath: {}", importer.GetErrorString(), path);

			return modelData;
		}

		m_fileDirectory = path.substr(0, path.find_first_of('/'));
		M_ProcessNode(scene->mRootNode, scene, *modelData);


		MNEMOSY_INFO("Loaded model from file: {}", path);

		return modelData;
	}


	void ModelLoader::M_ProcessNode(aiNode* node, const aiScene* scene, ModelData& modelData)
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

	MeshData ModelLoader::M_ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<VertexData> vertices;
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VertexData vertex;
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


		// TODO: create on the heap
		//MeshData meshData;
		//MeshData* meshData = new MeshData();
		std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>(); // !!! idk this might not work

		M_SetupMesh(*meshData, vertices, indices);

		return *meshData;
	}


	void ModelLoader::M_SetupMesh(MeshData& _meshData, std::vector<VertexData> Vertecies, std::vector<unsigned int> Indecies)
	{
		glGenVertexArrays(1, &_meshData.vertexArrayObject);
		glGenBuffers(1, &_meshData.vertexBufferObject);
		glGenBuffers(1, &_meshData.elementBufferObject);

		glBindVertexArray(_meshData.vertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, _meshData.vertexBufferObject);

		glBufferData(GL_ARRAY_BUFFER, Vertecies.size() * sizeof(VertexData), &Vertecies[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _meshData.elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indecies.size() * sizeof(unsigned int), &Indecies[0], GL_STATIC_DRAW);


		//positions
			// attribPointer index,someBool,somebool, how much bytes,offset in array in bytes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);

		// normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));

		// Tangents
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, tangent));

		// Bi Tangents
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, bitangent));

		// color 
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));

		// texCoord / uvs
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoords));

		glBindVertexArray(0);

		_meshData.vertecies = Vertecies;
		_meshData.indecies = Indecies;
	}

}