#pragma once
#include <unordered_map>
#include <Ouroboros/ECS/GameObject.h>
namespace oo
{

	class PrefabManager
	{
	public:
		PrefabManager();
		~PrefabManager();
		static void MakePrefab(std::shared_ptr<oo::GameObject> go);
	private:

	};

};