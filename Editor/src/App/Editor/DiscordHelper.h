#pragma once
#include <discord.h>
#include "App/Editor/Events/LoadProjectEvents.h"
#include "App/Editor/Events/LoadSceneEvent.h"
struct DiscordState {
	std::unique_ptr<discord::Core> core;
};
class DiscordHelper
{
public:
	DiscordHelper();
	~DiscordHelper();
	void Update();
private:
	void UpdateStatus(LoadProjectEvent* e);
	void UpdateStatus(LoadSceneEvent* e);
private:
	inline static constexpr discord::ClientId clientid = 1032685306828304485;
	discord::Activity m_current_activity{};
	DiscordState state;
	bool ready = false;
};
