/************************************************************************************//*!
\file           MeshModel.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a mesh object and resources for use on CPU and GPU

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "MeshModel.h"
#include "VulkanDevice.h"

#include <stdexcept>



void gfxModel::destroy(VkDevice device)
{

	auto DFS = [&](auto&& func, oGFX::BoneNode* pBoneNode) -> void
	{		
		if (pBoneNode == nullptr) return;
		// Recursion through all children nodes
		for (unsigned i = 0; i < pBoneNode->mChildren.size(); i++)
		{
			func(func, pBoneNode->mChildren[i]);
		}
		delete pBoneNode;
	};

	if (skeleton)
	{
		DFS(DFS, skeleton->m_boneNodes);
	}

}

void ModelFileResource::ModelSceneLoad(const aiScene* scene, 
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

void ModelFileResource::ModelBoneLoad(const aiScene* scene, const aiNode& node, uint32_t vertOffset)
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
						//idx = skeleton->m_boneNodes.size();
						//strToBone[name] = static_cast<uint32_t>(idx);
						//BoneOffset bo;
						//bo.transform = aiMat4_to_glm(bone->mOffsetMatrix);
						//boneOffsets.emplace_back(bo);
						//bones.push_back({});
					}

					std::cout << "Mesh :" << aimesh->mName.C_Str() << std::endl;
					std::cout << "Bone weights:" << bone->mNumWeights << std::endl;
					for (size_t y = 0; y < bone->mNumWeights; y++)
					{
						//auto& weight = bone->mWeights[y];
						//auto& vertWeight = boneWeights[weight.mVertexId];
						//auto& vertex = vertices[static_cast<size_t>(weight.mVertexId) + vertOffset + sumVerts];
						//
						//auto bNum = vertex.boneWeights++;
						////assert(bNum < 4); // CANNOT SUPPORT MORE THAN 4 BONES
						//if (bNum < 4)
						//{
						//	vertWeight.boneWeights[bNum] = weight.mWeight;
						//	vertWeight.boneIdx[bNum] = static_cast<uint32_t>(idx);
						//}
						//else
						//{
						//	std::cout << "Tried to load bones: " << bNum << std::endl;
						//}
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

ModelFileResource::~ModelFileResource()
{
	delete sceneInfo;
}	

oGFX::BoneNode* CopyBoneNode(const oGFX::BoneNode* rhs) {
	if (rhs == nullptr) return nullptr;

	auto bone = new oGFX::BoneNode();
	bone->mbIsBoneNode = rhs->mbIsBoneNode;
	bone->mModelSpaceGlobal = rhs->mModelSpaceGlobal;
	bone->mModelSpaceLocal = rhs->mModelSpaceLocal;
	bone->mName = rhs->mName;
	bone->m_BoneIndex = rhs->m_BoneIndex;
	
	bone->mChildren.reserve(rhs->mChildren.size());
	for (auto& child: rhs->mChildren)
	{
		auto newChild = bone->mChildren.emplace_back(CopyBoneNode(child));
		newChild->mpParent = bone;
	}
	return bone;
}

oGFX::CPUSkeletonInstance * oGFX::CreateCPUSkeleton(const Skeleton * rhs)
{
	if (rhs == nullptr)
		return nullptr;

	auto* skel = new oGFX::CPUSkeletonInstance();
	skel->m_boneNodes = CopyBoneNode(rhs->m_boneNodes);

	return skel;
}
