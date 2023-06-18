/************************************************************************************//*!
\file           Animation.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
An animation asset created via editor or imported from fbx to be referenced by nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"
#include "AnimationTimeline.h"


namespace oo::Anim
{
	struct Animation
	{
		friend class AnimationSystem;

		
		static std::unordered_map< std::string, size_t> name_to_ID;
		static std::map<size_t, Animation> animation_storage;


		std::string name{ "Unnamed Animation" };

		std::vector<ScriptEvent> events{};

		std::vector<Timeline> timelines{};

		bool looping{ false };

		float animation_length{ 0.f };

		UID animation_ID{ internal::generateUID() };

		//stored as float cos assimp loads it as double

		float frames_per_second{60ull};
		//float total_frames{ 0ull };

		float TimeFromFrame(size_t frame);

		RTTR_ENABLE();

	private:


		static std::vector<Animation*> LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource);
		static Animation* AddAnimation(Animation&& anim);
		static void RemoveAnimation(std::string const& name);

		static Animation ExtractAnimation(float start_time, float end_time, Animation& anim, bool& result);
		static Animation ExtractAnimation(size_t start_frame, size_t end_frame, Animation& anim, bool& result);

		static std::mutex animation_containers_mutex;
	};


}