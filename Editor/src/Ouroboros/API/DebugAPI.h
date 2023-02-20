/************************************************************************************//*!
\file           DebugAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to call any functions that help with outputting debug informations,
                like debug messages, or even lines in the world

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Core/Log.h"
#include "OO_Vulkan/src/DebugDraw.h"

namespace oo
{
    SCRIPT_API void Log(const char* filename, int lineNumber, const char* msg)
    {
        LOG_SCRIPT_TRACE(filename, lineNumber, msg);
    }

    SCRIPT_API void LogInfo(const char* filename, int lineNumber, const char* msg)
    {
        LOG_SCRIPT_INFO(filename, lineNumber, msg);
    }

    SCRIPT_API void LogWarning(const char* filename, int lineNumber, const char* msg)
    {
        LOG_SCRIPT_WARN(filename, lineNumber, msg);
    }

    SCRIPT_API void LogError(const char* filename, int lineNumber, const char* msg)
    {
        LOG_SCRIPT_ERROR(filename, lineNumber, msg);
    }

    SCRIPT_API void LogCritical(const char* filename, int lineNumber, const char* msg)
    {
        LOG_SCRIPT_CRITICAL(filename, lineNumber, msg);
    }

    SCRIPT_API void Debug_DrawLine(ScriptValue::vec3_type p0, ScriptValue::vec3_type p1)
    {
        DebugDraw::AddLine(p0, p1);
    }

    SCRIPT_API void Debug_DrawLine_Color(ScriptValue::vec3_type p0, ScriptValue::vec3_type p1, Color color)
    {
        DebugDraw::AddLine(p0, p1, color);
    }

    SCRIPT_API void Debug_DrawWireCube(ScriptValue::vec3_type center, ScriptValue::vec3_type size)
    {
        glm::vec3 min = static_cast<glm::vec3>(center) - (static_cast<glm::vec3>(size) / 2.0f);
        glm::vec3 max = static_cast<glm::vec3>(center) + (static_cast<glm::vec3>(size) / 2.0f);
        DebugDraw::AddAABB({ min, max });
    }

    SCRIPT_API void Debug_DrawWireCube_Color(ScriptValue::vec3_type center, ScriptValue::vec3_type size, Color color)
    {
        glm::vec3 min = static_cast<glm::vec3>(center) - (static_cast<glm::vec3>(size) / 2.0f);
        glm::vec3 max = static_cast<glm::vec3>(center) + (static_cast<glm::vec3>(size) / 2.0f);
        DebugDraw::AddAABB({ min, max }, color);
    }

    SCRIPT_API void Debug_DrawWireSphere(ScriptValue::vec3_type center, float radius)
    {
        DebugDraw::AddSphere({ center, radius });
    }

    SCRIPT_API void Debug_DrawWireSphere_Color(ScriptValue::vec3_type center, float radius, Color color)
    {
        DebugDraw::AddSphere({ center, radius }, color);
    }

    SCRIPT_API void Debug_DrawArrow(ScriptValue::vec3_type p0, ScriptValue::vec3_type p1)
    {
        DebugDraw::AddArrow(p0, p1);
    }

    SCRIPT_API void Debug_DrawArrow_Color(ScriptValue::vec3_type p0, ScriptValue::vec3_type p1, Color color)
    {
        DebugDraw::AddArrow(p0, p1, color);
    }
}