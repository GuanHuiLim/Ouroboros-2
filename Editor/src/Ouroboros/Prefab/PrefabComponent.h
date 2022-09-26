/************************************************************************************//*!
\file           PrefabComponent.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          simple prefab compnent for the root prefab to hold

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
		RTTR_ENABLE();
	private:
	};

};
