#include "Include/Graphics/ModelLoader.h"

#include "Include/Core/Log.h"

#include <glad/glad.h>
#include <memory>
#include <vector>

namespace mnemosy::graphics
{
	// public
	ModelData* ModelLoader::LoadModelDataFromFile(const std::string& path)
	{
		return	M_LoadModel(path);
	}

	// private
	ModelData* ModelLoader::M_LoadModel(const std::string& path)
	{

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace );
		//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// could use several options such as | aiProcess_GenNormals, aiProcess_OptimizeMeshes,aiProcess_SplitLargeMeshes,aiProcess_FlipUVs
		// importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals );

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			MNEMOSY_ERROR("Faild to load Model from file: {} \n Filepath: {}", importer.GetErrorString(), path);

			importer.FreeScene();
			ModelData* modelData = new ModelData();
			return modelData;
		}

		ModelData* modelData = new ModelData();
		M_ProcessNode(scene->mRootNode, scene, *modelData);

		importer.FreeScene();

		MNEMOSY_DEBUG("Loaded model from file: {}", path);
		return modelData;
	}


	void ModelLoader::M_ProcessNode(const aiNode* node, const aiScene* scene, ModelData& modelData)
	{

		// recursavly process all the node's sub meshes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			//aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			// !! Somthing broken with the import her.
			// semes to work for at least single mesh files..
			aiMesh* mesh = scene->mMeshes[i];

			modelData.meshes.push_back(M_ProcessMesh(mesh, scene));

			M_ProcessNode(node->mChildren[i], scene, modelData);
		}
	}

	MeshData ModelLoader::M_ProcessMesh(const aiMesh* mesh, const aiScene* scene)
	{
		std::vector<VertexData> vertices;
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VertexData vertex;
			// process data

			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			//vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

			if (mesh->mColors[0])
			{
				vertex.color = glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
			}
			else
			{
				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // default white
			}

			// checking if texCoords Exist
			if (mesh->mTextureCoords[0])
			{
				vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
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

		//MeshData meshData;
		//MeshData* meshData = new MeshData();
		std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>(); // !!! idk this might not work
		M_SetupMesh(*meshData, vertices, indices);

		vertices.clear();
		indices.clear();

		return *meshData;
	}


	void ModelLoader::M_SetupMesh(MeshData& _meshData, const std::vector<VertexData>& Vertecies, const std::vector<unsigned int>& Indecies)
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

		// // Bi Tangents
		// glEnableVertexAttribArray(3);
		// glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, bitangent));

		// color
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));

		// texCoord / uvs
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoords));

		glBindVertexArray(0);

		_meshData.vertecies = Vertecies;
		_meshData.indecies = Indecies;
	}

} // !mnemosy::graphics
