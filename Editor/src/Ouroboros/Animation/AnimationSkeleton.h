/************************************************************************************//*!
\file           AnimationSkeleton.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
contains pose data for Animations and 
Animation Transitions for a gameobject object

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

#include <unordered_map>
namespace oo::Anim
{
	class SkeletonBone
	{
		friend class AnimationSkeleton;
		friend class Pose;
		UID boneID{ internal::invalid_ID };
		UUID gameobjectUUID{};
		bool changed{ false };
		
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale	  { 1.0f, 1.0f, 1.0f };
	};
	//a collection of bones and their transformations
	class Pose
	{
		friend class AnimationSkeleton;
		std::unordered_map<size_t, uint> uid_to_boneIndex{};
		std::vector<SkeletonBone> bones{};

		inline SkeletonBone& GetBone(UID boneID)
		{
			return bones[uid_to_boneIndex[boneID]];
		}

		inline SkeletonBone& EnsureBone(UID boneID)
		{
			if (uid_to_boneIndex.contains(boneID) == false)
			{
				uid_to_boneIndex[boneID] = bones.size();
				bones.emplace_back();
				bones.back().boneID = boneID;
			}
			return GetBone(boneID);
		}

		inline void SetBoneData(UID boneID, UUID gameobjectUUID)
		{
			EnsureBone(boneID).gameobjectUUID = gameobjectUUID;
		}
	};
	//a collection of poses that can be blended and applied to their corresponding gameobjects
	class AnimationSkeleton
	{
		friend class AnimationSystem;
		std::vector<Pose> poses{3};
		rttr::property transform_Position_property { rttr::type::get<TransformComponent>().get_property("Position") };
		rttr::property transform_Quaternion_property { rttr::type::get<TransformComponent>().get_property("Quaternion") };
		rttr::property transform_Scaling_property { rttr::type::get<TransformComponent>().get_property("Scaling") };
		

		inline SkeletonBone& CurrentPoseBone(UID boneID)
		{
			return poses[CURRENT_POSE_INDEX].GetBone(boneID);
		}

		inline SkeletonBone& PoseBone(uint pose_index, UID boneID)
		{
			return poses[pose_index].GetBone(boneID);
		}
		
		inline void Apply_Pose_To_Gameobjects(uint pose_index, Scene& scene);
	public:
		static constexpr uint CURRENT_POSE_INDEX = 0;
		static constexpr uint NEXT_POSE_INDEX = 1;
		static constexpr uint OUTPUT_POSE_INDEX = 2;
		
		
		void SetCurrentPose_Bone_Quaternion_property(UID boneID, glm::quat const& rotation);
		void SetCurrentPose_Bone_vec3_property(UID boneID, rttr::property prop, glm::vec3 const& value);
		
		void SetPose_Bone_Quaternion_property(uint pose_index, UID boneID, glm::quat const& rotation);
		void SetPose_Bone_vec3_property(uint pose_index, UID boneID, rttr::property prop, glm::vec3 const& value);

		/*
		pose_index -> bone to set the value of
		blend_pose_index -> bone to blend with
		blend_weight -> weight of the blend for the pose at pose_index
		*/
		void SetPoseBlended_Bone_Quaternion_property(uint pose_index, uint blend_pose_index, float blend_weight, UID boneID, glm::quat const& rotation);
		void SetPoseBlended_Bone_vec3_property(uint pose_index, uint blend_pose_index, float blend_weight, UID boneID, rttr::property prop, glm::vec3 const& value);

		
		void Apply_CurrentPose_To_Gameobjects(Scene& scene);
		void Apply_NextPose_To_Gameobjects(Scene& scene);
		void Apply_OutputPose_To_Gameobjects(Scene& scene);

		void CopyPose(uint from, uint to);
		
		//QUICK BLEND
		/*
		* blends the output pose with the next pose
		* blendFactor -> blend weight of the next pose
		*/
		void Blend_OutputPose_with_NextPose(float blendFactor);
		//void Apply_BlendedPose_To_Gameobject(Scene& scene, UUID gameobjectUUID);
		inline void SetBoneData(UID boneID, UUID gameobjectUUID)
		{
			for(auto& pose : poses)
				pose.SetBoneData(boneID, gameobjectUUID);
		}
		RTTR_ENABLE();
	};


	
}