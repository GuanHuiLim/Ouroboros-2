#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "App/Editor/Steam/SteamInterface.h"

namespace oo
{
    SCRIPT_API bool CheckAchievement(const char* name)
    {
        return SteamInterface::GetAchivement(name);
    }

    SCRIPT_API void UnlockAchievement(const char* name)
    {
        SteamInterface::SetAchivement(name);
    }

    SCRIPT_API void SetAchievementStat_Int(const char* name, int value)
    {
        SteamInterface::SetStats_INT(name, value);
    }

    SCRIPT_API int GetAchievementStat_Int(const char* name)
    {
        return SteamInterface::GetStat_INT(name);
    }

    SCRIPT_API void SetAchievementStat_Float(const char* name, float value)
    {
        SteamInterface::SetStats_FLOAT(name, value);
    }

    SCRIPT_API float GetAchievementStat_Float(const char* name)
    {
        return SteamInterface::GetStat_FLOAT(name);
    }
}