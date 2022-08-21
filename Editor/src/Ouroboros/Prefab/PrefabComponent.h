#pragma once
#include <filesystem>
#include <rttr/type>
namespace oo
{

	class PrefabComponent
	{
	public:
		PrefabComponent();
		~PrefabComponent();
		void Init(std::filesystem::path& p);
		std::filesystem::path prefab_filePath;
	private:
	};

};
