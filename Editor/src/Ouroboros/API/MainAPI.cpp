/************************************************************************************//*!
\file           MainAPI.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Mainly used to include all header files containing exported helper functions 
                that the C# scripts will use, so that these header files get built into the exe

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

// Include ALL API header files here, needs to be included in a cpp for them to be included in the build

#include "ApplicationAPI.h"
#include "DebugAPI.h"
#include "GameObjectAPI.h"
#include "InputAPI.h"
#include "SceneManagerAPI.h"
#include "ScriptingAPI.h"
#include "TransformAPI.h"
#include "AudioSourceAPI.h"

#include "PhysicsAPI.h"
#include "RigidbodyAPI.h"
#include "ColliderAPI.h"

#include "MeshRendererAPI.h"
#include "LightAPI.h"
#include "ParticleEmitterAPI.h"
#include "AnimationAPI.h"
#include "RendererSettingsAPI.h"

#include "RectTransformAPI.h"
#include "UIAPI.h"
