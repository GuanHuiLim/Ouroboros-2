#pragma once
#include <steam_api.h>
#include <steam_api_common.h> //might not need this but its ok
class Steam_Callbacks 
{
private:
	//broadcast a event typed "SteamEvent" with bool value -> true = overlay up
	STEAM_CALLBACK(Steam_Callbacks, OnGameOverlayActivated, GameOverlayActivated_t);
	//achivements
	STEAM_CALLBACK(Steam_Callbacks, OnUserStatsReceived, UserStatsReceived_t);
	STEAM_CALLBACK(Steam_Callbacks, OnUserStatsStored, UserStatsStored_t);
	STEAM_CALLBACK(Steam_Callbacks, OnAchievementStored,UserAchievementStored_t);
};
class SteamInterface
{
public:
	static void Init();
	static void Update();
	static void ShutDown();

	//strictly for internal use
	static void RequestStats();
	static void SetAchivementReady(bool ready) { achivements_ready = ready; };
	static const int64 GetAppID() { return m_iAppID; };

	//set achivements
	static void SetAchivement(const char* API_NAME);
	//get stats by name 
	static int GetStat_INT(const char* API_NAME);
	static float GetStat_FLOAT(const char* API_NAME);

	static void SetStats_INT(const char* API_NAME, int value);
	static void SetStats_FLOAT(const char* API_NAME, float value);
private:
private:
	inline static Steam_Callbacks* callback_ptr;//need to maintain it as the callback works on RAII
	inline static int64 m_iAppID = 0;
	
	inline static bool steam_ready = false;
	inline static bool achivements_ready = false;
	
	inline static bool updated_stats = false;

};
