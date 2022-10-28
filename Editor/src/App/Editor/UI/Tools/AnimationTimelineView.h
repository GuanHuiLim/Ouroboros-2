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
	oo::Anim::Animation* animation = nullptr;
	
	int currentKeyFrame = 0;

	int minimalLegendLength = 196;
	int lineStartOffset = 8;

	int pixelsPerFrame = 10;
	int majorLinePerLines = 5;

	ImVec2 timelineRegionMin;
	ImVec2 timelineRegionMax;

	int currentFrame = 0;
	int currentLegendWidth = 0;

	int visibleStartingFrame = 0;
	int visibleEndingFrame = 0;

	float accumulatedPanningDeltaX = 0.0f;
	bool isPanningTimeline = false;

	void DisplayAnimationTimeline(oo::AnimationComponent* _animator);
	void DrawTimeLine(oo::Anim::Animation* _animation, float headerYPadding, float headerHeight = 24.0f);
	void DrawToolbar(oo::Anim::Animation* _animation, oo::Anim::ProgressTracker::UpdateFn callback);

	int GetFrameFromTimelinePos(float pos);
	float GetTimelinePosFromFrame(int frame);

};
