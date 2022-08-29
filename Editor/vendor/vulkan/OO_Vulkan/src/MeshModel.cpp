#include "MeshModel.h"
#include "VulkanDevice.h"
#include <stdexcept>

MeshContainer::MeshContainer()
{
}

MeshContainer::MeshContainer(std::vector<Mesh> newMeshList)
{
	meshList = newMeshList;
	model = glm::mat4(1.0f);
}

size_t MeshContainer::getMeshCount()
{
	return meshList.size();
}

Mesh* MeshContainer::getMesh(size_t index)
{
	if (index >= meshList.size())
	{
		throw std::runtime_error("Attempted to access invad mesh index!");
	}

	return &meshList[index];
}

const glm::mat4& MeshContainer::getModel()
{
	return model;
}

void MeshContainer::setModel(glm::mat4 newModel)
{
	model = newModel;
}

void MeshContainer::destroyMeshModel()
{
	for (auto &mesh : meshList)
	{
		mesh.destroyBuffers();
	}
}


std::vector<std::string> MeshContainer::LoadMaterials(const aiScene *scene)
{
	// create 1:1 size list of textures
	std::vector <std::string> textureList(scene->mNumMaterials);

	// go through each material and copy its texxture file name( if it exists)
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		//get the material
		aiMaterial *material = scene->mMaterials[i];

		// Initialize the texture to empty string ( will be replaced if texture exists)
		textureList[i] = "";

		// Check for a diffuse texture (standard details)
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			//get path of the texture file
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path)== AI_SUCCESS)
			{
				// C:\users\jtk\documents\thing.obj
				// Cut off any directory information already present
				std::string str = std::string(path.data);
				size_t idx = str.rfind("\\");
				std::string filename = str.substr(idx + 1);

				textureList[i] = filename;
			}
		}
		//if (material->GetTextureCount(aiTextureType_NORMALS))
		//{
		//	//get path of the texture file
		//	aiString path;
		//	if (material->GetTexture(aiTextureType_NORMALS, 0, &path)== AI_SUCCESS)
		//	{
		//		// C:\users\jtk\documents\thing.obj
		//		// Cut off any directory information already present
		//		std::string str = std::string(path.data);
		//		size_t idx = str.rfind("\\");
		//		std::string filename = str.substr(idx + 1);
		//
		//		textureList[i] = filename;
		//	}
		//}
		//if (material->GetTextureCount(aiTextureType_SPECULAR))
		//{
		//	//get path of the texture file
		//	aiString path;
		//	if (material->GetTexture(aiTextureType_SPECULAR, 0, &path)== AI_SUCCESS)
		//	{
		//		// C:\users\jtk\documents\thing.obj
		//		// Cut off any directory information already present
		//		std::string str = std::string(path.data);
		//		size_t idx = str.rfind("\\");
		//		std::string filename = str.substr(idx + 1);
		//
		//		textureList[i] = filename;
		//	}
		//}
		//if (material->GetTextureCount(aiTextureType_UNKNOWN))
		//{
		//	//get path of the texture file
		//	aiString path;
		//	if (material->GetTexture(aiTextureType_UNKNOWN, 0, &path)== AI_SUCCESS)
		//	{
		//		// C:\users\jtk\documents\thing.obj
		//		// Cut off any directory information already present
		//		std::string str = std::string(path.data);
		//		size_t idx = str.rfind("\\");
		//		std::string filename = str.substr(idx + 1);
		//
		//		textureList[i] = filename;
		//	}
		//}
	}
	return textureList;
}

std::vector<Mesh> MeshContainer::LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool commandPool, aiNode *node, const aiScene *scene, std::vector<int> matToTex)
{
	std::vector<Mesh> meshList;

	//go through teach mesh at this node and create it , then add it to our meshlist
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		meshList.push_back(LoadMesh(newPhysicalDevice, newDevice, transferQueue, commandPool,
			scene->mMeshes[node->mMeshes[i]], scene, matToTex));
	}

	// Go through each node attached to this node and load it then append their meshes to this nod's mesh list
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		std::vector<Mesh> newList = LoadNode(newPhysicalDevice,  newDevice,  transferQueue,  commandPool,
			node->mChildren[i], scene, matToTex);
		meshList.insert(meshList.end(), newList.begin(), newList.end());
	}
	return meshList;
}

Mesh MeshContainer::LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
	aiMesh *mesh, const aiScene *scene, std::vector<int> matToTex)
{
	std::vector<oGFX::Vertex> vertices;
	std::vector<uint32_t> indices;

	// Resize vertex list to hold all vertices for mesh
	vertices.resize(mesh->mNumVertices);
	// Go through each vertex and copy it across to our vertices
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Set position
		vertices[i].pos = { mesh->mVertices[i].x ,mesh->mVertices[i].y,mesh->mVertices[i].z };

		// Set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].tex = { mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y };
		}
		else
		{
			//default value
			vertices[i].tex = { 0.0f,0.0f };
		}

		// Set colour to white for now
		//vertices[i].col = { 1.0f,1.0f,1.0f };
	}

	// Iterate over indices through ffaces and copy accross
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// get a face
		aiFace face = mesh->mFaces[i];

		// go through face's indices and add to list
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create a new mesh with details and return it
	Mesh newMesh = Mesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool,
		&vertices, &indices, matToTex[mesh->mMaterialIndex]);
	return newMesh;
}

void gfxModel::destroy(VkDevice device)
{
	vkDestroyBuffer(device, vertices.buffer, nullptr);
	vkFreeMemory(device, vertices.memory, nullptr);
	vkDestroyBuffer(device, indices.buffer, nullptr);
	vkFreeMemory(device, indices.memory, nullptr);
	for (auto& node : nodes)
	{
		delete node;
		node = nullptr;
	}
}

void gfxModel::loadNode(Node* parent,const aiScene* scene, const aiNode& node, uint32_t nodeIndex, std::vector<oGFX::Vertex>& vertices, std::vector<uint32_t>& indices)
{
	Node* newNode = new Node();
	newNode->parent = parent;
	newNode->index = nodeIndex;
	newNode->name = node.mName.C_Str();

	if (node.mNumChildren > 0)
	{
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			loadNode(newNode, scene, *node.mChildren[i], nodeIndex + static_cast<uint32_t>(i), vertices, indices);
		}
	}

	if (node.mNumMeshes > 0)
	{
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[node.mMeshes[i]];
			newNode->meshes.push_back( processMesh(aimesh, scene, vertices, indices));
		}
	}

	if (parent == nullptr)
	{
		nodes.push_back(newNode);	
	}
	else
	{
		parent->children.push_back(newNode);
	}
}

inline glm::vec3 aiVector3D_to_glm(const aiVector3D& v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

oGFX::Mesh* gfxModel::processMesh(aiMesh* aimesh, const aiScene* scene, std::vector<oGFX::Vertex>& vertices, std::vector<uint32_t>& indices)
{
	oGFX::Mesh* mesh = new oGFX::Mesh;
	mesh->vertexOffset  = static_cast<uint32_t>(vertices.size());
	mesh->indicesOffset = static_cast<uint32_t>(indices.size());
	mesh->vertexCount += aimesh->mNumVertices;
	vertices.reserve(vertices.size() + aimesh->mNumVertices);

	for (size_t i = 0; i < aimesh->mNumVertices; i++)
	{
		oGFX::Vertex vertex;
		const auto& aiVert = aimesh->mVertices[i];
		vertex.pos = aiVector3D_to_glm(aimesh->mVertices[i]);
		if (aimesh->HasTextureCoords(0)) // does the mesh contain texture coordinates?
		{
			vertex.tex = glm::vec2{ aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y };
		}
        if (aimesh->HasNormals())
        {
            vertex.norm = aiVector3D_to_glm(aimesh->mNormals[i]);
        }
        if (aimesh->HasTangentsAndBitangents())
        {
            vertex.tangent = aiVector3D_to_glm(aimesh->mTangents[i]);
        }
		if (aimesh->HasVertexColors(0))
		{
			const auto& color = aimesh->mColors[0][i];
			vertex.col = glm::vec4{ color.r, color.g, color.b, color.a };
		}

		vertices.emplace_back(vertex);
	}

	uint32_t indicesCnt{ 0 };
	indices.reserve(indices.size() + aimesh->mNumFaces * aimesh->mFaces[0].mNumIndices);

	for(uint32_t i = 0; i < aimesh->mNumFaces; i++)
	{
		const aiFace& face = aimesh->mFaces[i];
		indicesCnt += face.mNumIndices;
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	} 
	mesh->indicesCount = indicesCnt;

	if (aimesh->mMaterialIndex >= 0)
	{
		mesh->textureIndex = aimesh->mMaterialIndex;
	}

	return mesh;
}
