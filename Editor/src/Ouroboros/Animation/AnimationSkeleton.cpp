#include "pch.h"
#include "AnimationSkeleton.h"
#include "glm/common.hpp"


void oo::Anim::AnimationSkeleton::SetCurrentPose_Bone_Quaternion_property(UID boneID, glm::quat const& rotation)
{
	auto& bone = CurrentPoseBone(boneID);
	bone.rotation = rotation;
	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::SetCurrentPose_Bone_vec3_property(UID boneID, rttr::property prop, glm::vec3 const& value)
{
	auto& bone = CurrentPoseBone(boneID);
	//either its setting position or scale for vec3 type
	if (prop == transform_Position_property)
		bone.position = value;
	else
		bone.scale = value;

	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::SetPose_Bone_Quaternion_property(uint pose_index, UID boneID, glm::quat const& rotation)
{
	auto& bone = PoseBone(pose_index, boneID);
	bone.rotation = rotation;
	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::SetPose_Bone_vec3_property(uint pose_index, UID boneID, rttr::property prop, glm::vec3 const& value)
{
	auto& bone = PoseBone(pose_index, boneID);
	//either its setting position or scale for vec3 type
	if (prop == transform_Position_property)
		bone.position = value;
	else
		bone.scale = value;

	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::SetPoseBlended_Bone_Quaternion_property(uint pose_index, uint blend_pose_index, float blend_weight, UID boneID, glm::quat const& rotation)
{
	auto& bone = PoseBone(pose_index, boneID);
	auto& inputbone = PoseBone(blend_pose_index, boneID);
	bone.rotation = glm::slerp(inputbone.rotation, rotation, blend_weight);
	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::SetPoseBlended_Bone_vec3_property(uint pose_index, uint blend_pose_index, float blend_weight, UID boneID, rttr::property prop, glm::vec3 const& value)
{
	auto& bone = PoseBone(pose_index, boneID);
	auto& inputbone = PoseBone(blend_pose_index, boneID);
	//either its setting position or scale for vec3 type
	if (prop == transform_Position_property)
		bone.position = glm::mix(inputbone.position, value, blend_weight);
	else
		bone.scale = glm::mix(inputbone.scale, value, blend_weight);

	bone.changed = true;
}

void oo::Anim::AnimationSkeleton::Apply_CurrentPose_To_Gameobjects(Scene& scene)
{
	Apply_Pose_To_Gameobjects(CURRENT_POSE_INDEX, scene);
}

void oo::Anim::AnimationSkeleton::Apply_NextPose_To_Gameobjects(Scene& scene)
{
	Apply_Pose_To_Gameobjects(NEXT_POSE_INDEX, scene);
}

void oo::Anim::AnimationSkeleton::Apply_OutputPose_To_Gameobjects(Scene& scene)
{
	Apply_Pose_To_Gameobjects(OUTPUT_POSE_INDEX, scene);
}

void oo::Anim::AnimationSkeleton::CopyPose(uint from, uint to)
{
	auto& to_pose = poses[to];
	auto& from_bones = poses[from].bones;

	uint index{ 0ul };

	for (auto& bone : to_pose.bones)
	{
		bone = from_bones[index];
		++index;
	}

}

void oo::Anim::AnimationSkeleton::Blend_OutputPose_with_NextPose(float blendFactor)
{
	auto& outputPose = poses[OUTPUT_POSE_INDEX];
	auto& nextPose = poses[NEXT_POSE_INDEX];

	uint index = 0ul;
	for (auto& bone : outputPose.bones)
	{
		auto& nextBone = nextPose.bones[index];
		
		bone.position = glm::mix(bone.position, nextBone.position, blendFactor);
		bone.rotation = glm::slerp(bone.rotation, nextBone.rotation, blendFactor);
		bone.scale = glm::mix(bone.scale, nextBone.scale, blendFactor);
	}
}

void oo::Anim::AnimationSkeleton::Apply_Pose_To_Gameobjects(uint pose_index, Scene& scene)
{
	auto& pose = poses[pose_index];

	for (auto& bone : pose.bones)
	{
		if (bone.changed == false) continue;

		auto go = scene.FindWithInstanceID(bone.gameobjectUUID);
		auto& transform = go->GetComponent<oo::TransformComponent>();

		transform.SetPosition(bone.position);
		transform.SetOrientation(bone.rotation);
		transform.SetScale(bone.scale);

		bone.changed = false;
	}
}
