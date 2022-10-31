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


class AnimationTimelineView
{
public:

	void Show();

private:
	oo::AnimationComponent* animator = nullptr;
	oo::Anim::Node* node = nullptr;
	oo::Anim::Animation* animation = nullptr;
	oo::Anim::Timeline* timeline = nullptr;
	
	static int currentKeyFrame;
	static float unitPerFrame;
	static float currentTime;
	static bool playAnim;

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

	void DisplayAnimationTimeline(oo::AnimationComponent* _animator);
	void DrawToolbar();
	void DrawNodeSelector(oo::AnimationComponent* _animator);
	void DrawTimelineSelector(oo::Anim::Animation* _animation);
	void DrawTimeLine(oo::Anim::Animation* _animation, float headerYPadding, float headerHeight = 24.0f);
	void DrawKeyFrame(int _currentKeyFrame, ImU32 colour);

	int GetFrameFromTimelinePos(float pos);
	float GetTimelinePosFromFrame(int frame);

};
