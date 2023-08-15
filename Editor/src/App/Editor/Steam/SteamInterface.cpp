#include "pch.h"
#include "SteamInterface.h"
#include "Ouroboros/Core/Log.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/Events/SteamEvent.h"
void SteamInterface::Init()
{
	steam_ready = SteamAPI_Init();
	if (!steam_ready)
	{
		LOG_CORE_CRITICAL("steam not ready");
		return;
	}
	m_iAppID = SteamUtils()->GetAppID();
	callback_ptr = new Steam_Callbacks();
	RequestStats();
}

void SteamInterface::Update()
{
	if(steam_ready)
		SteamAPI_RunCallbacks();
	if (updated_stats)
	{
		SteamUserStats()->StoreStats();
		updated_stats = false;
	}
}

void SteamInterface::ShutDown()
{
	if (!steam_ready)
		return;
	SteamAPI_Shutdown();
	delete callback_ptr;
}
void SteamInterface::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return;
	}
	SteamUserStats()->RequestCurrentStats();
}
void SteamInterface::SetAchivement(const char* API_NAME)
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return;
	}
	if (achivements_ready)
	{
		SteamUserStats()->SetAchievement(API_NAME);
		if (!SteamUserStats()->StoreStats())
		{
			LOG_CORE_CRITICAL("failed to store stats");
		}
	}
}
int SteamInterface::GetStat_INT(const char* API_NAME)
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return -1;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return -1;
	}
	int value = 0;
	SteamUserStats()->GetStat(API_NAME, &value);
	return value;
}
float SteamInterface::GetStat_FLOAT(const char* API_NAME)
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return -1;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return -1;
	}
	float value = 0;
	SteamUserStats()->GetStat(API_NAME, &value);
	return value;
}
void SteamInterface::SetStats_INT(const char* API_NAME, int value)
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return;
	}
	updated_stats = true;
	SteamUserStats()->SetStat(API_NAME, value);
}
void SteamInterface::SetStats_FLOAT(const char* API_NAME, int value)
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return;
	}
	updated_stats = true;
	SteamUserStats()->SetStat(API_NAME, value);
}
void Steam_Callbacks::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
{
	SteamEvent se(pCallback->m_bActive);
	oo::EventManager::Broadcast<SteamEvent>(&se);
}
void Steam_Callbacks::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	if(SteamInterface::GetAppID() == pCallback->m_nGameID)
		SteamInterface::SetAchivementReady(pCallback->m_eResult);
	else 
	{
		char buffer[128];
		_snprintf(buffer, 128, "RequestStats - failed, %d\n", pCallback->m_eResult);
		OutputDebugStringA(buffer);
	}
}
void Steam_Callbacks::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	if (SteamInterface::GetAppID() == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
			OutputDebugStringA("Stored stats for Steam\n");
		}
		else
		{
			char buffer[128];
			_snprintf(buffer, 128, "StatsStored - failed, %d\n", pCallback->m_eResult);
			OutputDebugStringA(buffer);
		}
	}
}
void Steam_Callbacks::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (SteamInterface::GetAppID() == pCallback->m_nGameID)
	{
		OutputDebugStringA("Stored Achievement for Steam\n");
	}
}


