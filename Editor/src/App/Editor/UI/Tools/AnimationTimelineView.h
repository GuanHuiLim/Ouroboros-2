/************************************************************************************//*!
\file          AnimatorTimelineView.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 22, 2022
\brief         File Contains the declaration needed to create an Animator Timeline View
			   for the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <Ouroboros/Animation/Anim.h>
#include <Ouroboros/Animation/AnimationComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>
#include <Ouroboros/Scripting/ScriptComponent.h>


class AnimationTimelineView
{
public:
	void Show();

private:
	oo::Scene::go_ptr source_go;
	oo::AnimationComponent* animator = nullptr;
	oo::Anim::Node* node = nullptr;
	oo::Anim::Animation* animation = nullptr;
	oo::Anim::ScriptEvent* scriptevent = nullptr;
	float currentFrameTime = 0.0f;
	oo::Anim::Timeline* timeline = nullptr;
	oo::Anim::KeyFrame* keyframe = nullptr;
	std::vector<oo::ScriptValue::function_info> fnInfo;

	//to be used to apply data to gameObject, for previewing
	std::vector<oo::Anim::Timeline*> timelines;	//if there are multiple timelines that has a keyframe at that currentKeyFrame, store them both in here
	std::vector<oo::Anim::KeyFrame*> keyframes;	//Since there are multiple timeline that may have a keyframe at this currentKeyFrame, invoke both keyframes

	void DisplayAnimationTimeline(oo::AnimationComponent* _animator);
	void DisplayAnimationTimelineView(oo::AnimationComponent* _animator);
	void SelectAnimation(oo::AnimationComponent* _animator, 
						 std::map<size_t, oo::Anim::Group>& _group,
						 std::string& _animName, 
						 bool& _animOpen);
	void AnimationFrameTime(oo::Anim::Animation* _animation);
	void AnimationEventList(oo::Anim::Animation* _animation);
	void DisplayAnimationEventView(oo::Anim::Animation* _animation);
};

#ifdef LEGACY_CODE
class AnimationTimelineView
{
public:

	void Show();

private:
	oo::Scene::go_ptr source_go;
	oo::AnimationComponent* animator = nullptr;
	oo::Anim::Node* node = nullptr;
	oo::Anim::Animation* animation = nullptr;
	oo::Anim::ScriptEvent* scriptevent = nullptr;
	oo::Anim::Timeline* timeline = nullptr;
	oo::Anim::KeyFrame* keyframe = nullptr;
	std::vector<oo::ScriptValue::function_info> fnInfo;

	//to be used to apply data to gameObject, for previewing
	std::vector<oo::Anim::Timeline*> timelines;	//if there are multiple timelines that has a keyframe at that currentKeyFrame, store them both in here
	std::vector<oo::Anim::KeyFrame*> keyframes;	//Since there are multiple timeline that may have a keyframe at this currentKeyFrame, invoke both keyframes

	static int currentKeyFrame;
	static float unitPerFrame;
	static float currentTime;
	static bool playAnim;
	static bool opentimeline;

	int minimalLegendLength = 196;
	int lineStartOffset = 8;

	int pixelsPerFrame = 10;
	int majorLinePerLines = 5;

	ImVec2 timelineRegionMin;
	ImVec2 timelineRegionMax;

	int currentFrame = 0;
	int currentLegendWidth = 0;

	float visibleStartingFrame = 0.0f;
	float visibleEndingFrame = 0.0f;

	float accumulatedPanningDeltaX = 0.0f;
	bool isPanningTimeline = false;

	bool displayEventInspector = true;

	void DisplayAnimationTimeline(oo::AnimationComponent* _animator);
	void DisplayInspector();
	void DrawTimeLineInfo();
	void DrawToolbar(oo::AnimationComponent* _animator);
	void DrawNodeSelector(oo::AnimationComponent* _animator);
	void DrawTimeLine(oo::Anim::Animation* _animation, float headerYPadding, float headerHeight = 24.0f);
	void DrawTimeLineContent();
	void DrawTimeLineSelector(oo::GameObject* go);
	void DrawKeyFrame(int _currentKeyFrame, const ImVec4& colour, float ypos = 0.0f, const std::string& label = "");

	int GetFrameFromTimelinePos(float pos);
	float GetTimelinePosFromFrame(int frame);
};
#endif