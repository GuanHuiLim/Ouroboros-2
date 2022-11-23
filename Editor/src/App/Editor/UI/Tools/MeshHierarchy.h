/************************************************************************************//*!
\file          MeshHierarchy.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Declarations for MeshHierarchy 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Asset/Asset.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "OO_Vulkan/src/Node.h"
#include "Ouroboros/ECS/GameObject.h"
class MeshHierarchy
{
public:
	struct MeshHierarchyDragDropData
	{
		Node* data;
		oo::AssetID id;
	};
	MeshHierarchy();
	~MeshHierarchy();
	void OpenFileCallBack(OpenFileEvent* e);
	void Show();
	static void CreateObject(Node* starting_node,oo::AssetID asset_id );
private:
	static std::shared_ptr<oo::GameObject> CreateSkeleton(ModelFileResource* resource, uint32_t gfx_ID, oo::UUID uid);
	oo::AssetID m_current_id = 0;
};