/************************************************************************************//*!
\file           Input.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares Input handler for windows

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#ifndef KEYCODES_H
#define KEYCODES_H

#include <cstdint>
#include "keycodes.h"

#include "MathCommon.h"

struct Input
{
static constexpr uint32_t NUM_KEYS = 1024;


static bool keysTriggered[NUM_KEYS];
static bool keysHeld[NUM_KEYS];
static bool keysRelease[NUM_KEYS];
static glm::vec2 mousePos;
static glm::vec2 mouseChange;

static short wheelDelta;

enum MouseButton
{
	left = 0,
	right = 1,
	middle = 2,
};


static bool mouseButtonTriggered[3];
static bool mouseButtonHeld[3];
static bool mouseButtonRelease[3]; 

static void Begin();

static bool GetKeyTriggered(int32_t key);

static bool GetKeyHeld(int32_t key);

static bool GetKeyReleased(int32_t key);

static void HandleMouseMove(int32_t x, int32_t y);

static glm::vec2 GetMouseDelta();
static glm::vec2 GetMousePos();

static float GetMouseWheel();

static bool GetMouseTriggered(int32_t key);
static bool GetMouseHeld(int32_t key);
static bool GetMouseReleased(int32_t key);


}; // namespace Input


#endif
