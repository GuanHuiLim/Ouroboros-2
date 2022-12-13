#include "pch.h"
#include "DiscordHelper.h"
#include "Ouroboros/EventSystem/EventManager.h"
DiscordHelper::DiscordHelper()
{
	discord::Core* core{};
	discord::Core::Create(clientid, DiscordCreateFlags_NoRequireDiscord, &core);
	state.core.reset(core);
	if (!state.core)
	{
		LOG_CORE_ERROR("Failed to instantiate Discord!");
		return;
	}
	ready = true;

	//first activity
	discord::PartySize ptsz;
	ptsz.SetMaxSize(5);
	ptsz.SetCurrentSize(1);
	m_current_activity.GetParty().GetSize() = ptsz;
	m_current_activity.SetDetails("Project Selection");
	m_current_activity.SetState("Idle");
	m_current_activity.GetTimestamps().SetStart(time(0));
	m_current_activity.GetAssets().SetSmallImage("arcadia_logo");
	m_current_activity.GetAssets().SetSmallText("Arcadia");
	m_current_activity.GetAssets().SetLargeImage("editoricon_1");
	m_current_activity.GetAssets().SetLargeText("Ouroboros Engine");
	
	state.core->ActivityManager().UpdateActivity(m_current_activity, 0);

	oo::EventManager::Subscribe<DiscordHelper, LoadProjectEvent>(this, &DiscordHelper::UpdateStatus);
	oo::EventManager::Subscribe<DiscordHelper, LoadSceneEvent>(this, &DiscordHelper::UpdateStatus);
}

DiscordHelper::~DiscordHelper()
{
}

void DiscordHelper::Update()
{
	if (ready)
		state.core->RunCallbacks();
}

void DiscordHelper::UpdateStatus(LoadProjectEvent* e)
{
	if (ready == false)
		return;

	m_current_activity.SetDetails(std::filesystem::path(e->m_projectPath).stem().string().c_str());
	state.core->ActivityManager().UpdateActivity(m_current_activity, 0);
}

void DiscordHelper::UpdateStatus(LoadSceneEvent* e)
{
	if (ready == false)
		return;

	m_current_activity.SetState(std::filesystem::path(e->m_scene->GetFilePath()).stem().string().c_str());
	state.core->ActivityManager().UpdateActivity(m_current_activity, 0);
}
