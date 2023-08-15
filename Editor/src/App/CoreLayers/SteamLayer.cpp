#include <pch.h>
#include "SteamLayer.h"
#include <App/Editor/Steam/SteamInterface.h>

void SteamLayer::OnAttach()
{
	SteamInterface::Init();
}

void SteamLayer::OnUpdate()
{
	SteamInterface::Update();
}

void SteamLayer::OnDetach()
{
	SteamInterface::ShutDown();
}
