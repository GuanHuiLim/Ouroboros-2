/************************************************************************************//*!
\file           Anim.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Definitions for animation related classes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Anim.h"
#include "AnimationInternal.h"
#include "AnimationSystem.h"
#include "Archetypes_Ecs/src/A_Ecs.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "Project.h"
#include "App/Editor/UI/Tools/MeshHierarchy.h"

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <rttr/registration>
#include <random>
namespace oo::Anim::internal
{

} //namespace oo::Anim::internal

namespace oo::Anim
{
	/*-------------------------------
	Timeline
	-------------------------------*/
	Timeline::Timeline(TimelineInfo const& info)
		: type{ info.type }
		, rttr_property{info.rttr_property}
		, rttr_type{ info.rttr_property.get_type()}
		, name{info.timeline_name}
		, component_hash{info.component_hash}
		, children_index{ info.children_index }
	{
		get_componentFn = Ecs::ECSWorld::get_component_Fn(component_hash);

		//verify able to retrieve the component info
		assert(get_componentFn != nullptr);
	}
	/*Timeline::Timeline(TYPE _type, DATATYPE _datatype, std::string const _name)
		: type{_type}
		, datatype{_datatype}
		, name{_name}
		, rttr_type{rttr::type::get<TransformComponent>()}
	{
		rttr::property::get_type("")
	}*/

	

	
	/*-------------------------------
	AnimationComponent
	-------------------------------*/
}
