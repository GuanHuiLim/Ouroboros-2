#pragma once
#include "Ouroboros/Asset/Asset.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "OO_Vulkan/src/Node.h"

//test
#include <filesystem>
class MeshHierarchy
{
public:
	MeshHierarchy();
	~MeshHierarchy();
	void OpenFileCallBack(OpenFileEvent* e);
	void Show();
private:
	void CreateObject(Node* starting_node,oo::AssetID asset_id );
	oo::AssetID m_current_id = 0;
	std::filesystem::path temp;
};
