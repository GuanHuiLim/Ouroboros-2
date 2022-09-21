#pragma once
#include "Ouroboros/Asset/Asset.h"
#include "App/Editor/Events/OpenFileEvent.h"


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
	oo::AssetID m_current_id = 0;
	std::filesystem::path temp;
};
