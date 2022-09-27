#include "MeshModel.h"
#include "VulkanDevice.h"

#include <stdexcept>

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
		std::cerr << "Attempted to access invalid mesh index!" << std::endl;
		throw std::runtime_error("Attempted to access invalid mesh index!");
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


inline glm::vec3 aiVector3D_to_glm(const aiVector3D& v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

inline glm::mat4 aiMat4_to_glm(const aiMatrix4x4& m)
{
	return glm::mat4{
		{ m.a1, m.b1, m.c1, m.d1 },
		{ m.a2, m.b2, m.c2, m.d2 },
		{ m.a3, m.b3, m.c3, m.d3 },
		{ m.a4, m.b4, m.c4, m.d4 },	};
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
				// Cut off any directory information already present
				std::string str = std::string(path.data);
				size_t idx = str.rfind("\\");
				std::string filename = str.substr(idx + 1);

				textureList[i] = filename;
			}
		}
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

	delete mesh;

	for (auto& node : nodes)
	{
		delete node;
		node = nullptr;
	}
}

void gfxModel::loadNode(Node* parent,const aiScene* scene, const aiNode& node, uint32_t nodeIndex,
	ModelData& cpuModel)
{
	Node* newNode = new Node();
	newNode->parent = parent;
	newNode->name = node.mName.C_Str();

	if (node.mNumChildren > 0)
	{
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			loadNode(newNode, scene, *node.mChildren[i], nodeIndex + static_cast<uint32_t>(i), cpuModel);
		}
	}

	if (node.mNumMeshes > 0)
	{
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[node.mMeshes[i]];
			//newNode->meshes.push_back( processMesh(aimesh, scene, cpuModel.vertices, cpuModel.indices));

		
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

void offsetUpdateHelper(Node* parent, uint32_t& meshcount, uint32_t idxOffset, uint32_t vertOffset)
{
	//for (auto& node : parent->children)
	//{
	//	offsetUpdateHelper(node, meshcount, idxOffset, vertOffset);
	//}
	//for (auto& mesh :parent->meshes)
	//{
	//	mesh->indicesOffset += idxOffset;
	//	mesh->vertexOffset += vertOffset;
	//
	//	++meshcount;
	//}		
}

void gfxModel::updateOffsets(uint32_t idxOffset, uint32_t vertOffset)
{
	mesh->indicesOffset += idxOffset;
	mesh->vertexOffset += vertOffset;
	//for (auto& node : nodes)
	//{
	//	offsetUpdateHelper(node,this->meshCount, idxOffset, vertOffset);
	//}
}

oGFX::Mesh* gfxModel::processMesh(aiMesh* aimesh, const aiScene* scene, ModelData& mData)
{
	oGFX::Mesh* mesh = new oGFX::Mesh;

	auto& vertices = mData.vertices;
	auto& indices = mData.indices;

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

	if (scene->HasAnimations() &&aimesh->HasBones())
	{
		for (size_t x = 0; x < aimesh->mNumBones; x++)
		{
			auto& bone = aimesh->mBones[x];
			std::string name(bone->mName.C_Str());

			auto iter = mData.strToBone.find(name);
			if (iter != mData.strToBone.end())
			{
				// Duplicate bone name!
				//assert(false);
			}
			else
			{
				auto sz = mData.bones.size();
				mData.bones.push_back({});
				BoneOffset bo;
				bo.transform = aiMat4_to_glm(bone->mOffsetMatrix);
				mData.boneOffsets.emplace_back(bo);
				mData.strToBone[name] = static_cast<uint32_t>(sz);
			}
			//cpuModel.boneWeights.resize()

			std::cout << "Bone :" << name << std::endl;
			std::cout << "Bone weights:" << bone->mNumWeights << std::endl;
			for (size_t y = 0; y < bone->mNumWeights; y++)
			{
				//std::cout << "\t v" << bone->mWeights[y].mVertexId << ":" << bone->mWeights[y].mWeight << std::endl;
			}
		}

	}

	uint32_t indicesCnt{ 0 };
	indices.reserve(indices.size() + size_t(aimesh->mNumFaces) * aimesh->mFaces[0].mNumIndices);

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

void gfxModel::loadNode(const aiScene* scene, const aiNode& node, Node* targetparent, ModelData& cpuModel, glm::mat4 accMat)
{
	//glm::mat4 transform;
	//// if node has meshes, create a new scene object for it
	//if( node.mNumMeshes > 0)
	//{
	//	SceneObjekt newObject = new SceneObject;
	//	targetParent.addChild( newObject);
	//	// copy the meshes
	//	CopyMeshes( node, newObject);
	//	// the new object is the parent for all child nodes
	//	parent = newObject;
	//	transform.SetUnity();
	//} else
	//{
	//	// if no meshes, skip the node, but keep its transformation
	//	parent = targetparent;
	//	transform = aiMat4_to_glm(node.mTransformation) * accMat;
	//}
	//// continue for all child nodes
	//for( all node.mChildren)
	//	CopyNodesWithMeshes( node.mChildren[a], parent, transform);
}

void ModelData::ModelSceneLoad(const aiScene* scene, 
	const aiNode& node,
	Node* parent,
	const glm::mat4 accMat)
{
	std::vector<Node*> curNodes;
	auto xform = aiMat4_to_glm(node.mTransformation) * accMat;
	Node* targetParent = parent;

	if (parent == nullptr)
	{
		sceneInfo = new Node();
		targetParent = sceneInfo;
		targetParent->name = "MdlSceneRoot";
	}
	if (node.mNumMeshes > 0)
	{
		this->sceneMeshCount += node.mNumMeshes;
		curNodes.resize(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			curNodes[i] = new Node();
			curNodes[i]->parent = targetParent;
			curNodes[i]->meshRef = node.mMeshes[i];
			curNodes[i]->name = node.mName.C_Str();
			curNodes[i]->transform = xform;
		}
		// setup nodes
		auto child = targetParent->children.insert(std::end(targetParent->children),std::begin(curNodes), std::end(curNodes));
		targetParent = *child;
	}		
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		ModelSceneLoad(scene, *node.mChildren[i], targetParent, xform);
	}
}

void ModelData::ModelBoneLoad(const aiScene* scene, const aiNode& node, uint32_t vertOffset)
{	
	uint32_t sumVerts{};
	if (node.mNumMeshes > 0)
	{
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			auto & aimesh = scene->mMeshes[node.mMeshes[i]];
			
			if (aimesh->HasBones())
			{
				for (size_t x = 0; x < aimesh->mNumBones; x++)
				{
					auto& bone = aimesh->mBones[x];
					std::string name(bone->mName.C_Str());

					size_t idx = 0;
					auto iter = strToBone.find(name);
					if (iter != strToBone.end())
					{
						// Duplicate bone name!
						idx = static_cast<size_t>(iter->second);
						//assert(false);
					}
					else
					{
						idx = bones.size();
						strToBone[name] = static_cast<uint32_t>(idx);
						BoneOffset bo;
						bo.transform = aiMat4_to_glm(bone->mOffsetMatrix);
						boneOffsets.emplace_back(bo);
						bones.push_back({});
					}

					std::cout << "Mesh :" << aimesh->mName.C_Str() << std::endl;
					std::cout << "Bone weights:" << bone->mNumWeights << std::endl;
					for (size_t y = 0; y < bone->mNumWeights; y++)
					{
						auto& weight = bone->mWeights[y];
						auto& vertWeight = boneWeights[weight.mVertexId];
						auto& vertex = vertices[static_cast<size_t>(weight.mVertexId) + vertOffset + sumVerts];

						auto bNum = vertex.boneWeights++;
						//assert(bNum < 4); // CANNOT SUPPORT MORE THAN 4 BONES
						if (bNum < 4)
						{
							vertWeight.boneWeights[bNum] = weight.mWeight;
							vertWeight.boneIdx[bNum] = static_cast<uint32_t>(idx);
						}
						else
						{
							std::cout << "Tried to load bones: " << bNum << std::endl;
						}
						//std::cout << "\t v" << bone->mWeights[y].mVertexId << ":" << bone->mWeights[y].mWeight << std::endl;
					}
				}

			}			
			sumVerts += aimesh->mNumVertices;
		}
	}		
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		ModelBoneLoad(scene, *node.mChildren[i], vertOffset);
	}
}

ModelData::~ModelData()
{
	delete sceneInfo;
}	
	

	

