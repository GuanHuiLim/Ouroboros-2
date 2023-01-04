#include "pch.h"
#include "Keylogging.h"
#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Core/Timer.h"
#include "Ouroboros/Core/Application.h"
#include <imgui/imgui.h>
#include <sdl2/SDL.h>
#include "App/Editor/Events/ToolbarButtonEvent.h"
#include "Ouroboros/EventSystem/EventManager.h".
#include "App/Editor/UI/Tools/WarningMessage.h"
#define DEBUG_KEYBOARD
KeyLogging::KeyLogging()
{
	Reset();
	m_mousePosition.reserve(100000);
	m_actionDown.reserve(256);
	m_actionUp.reserve(256);
	m_mousePressed.reserve(256);
	m_mousestate[oo::input::MouseCode::Button0] = false;
	m_mousestate[oo::input::MouseCode::Button1] = false;
	m_mousestate[oo::input::MouseCode::Button2] = false;
	m_mousestate[oo::input::MouseCode::Button3] = false;
	m_mousestate[oo::input::MouseCode::Button4] = false;
	m_mousestate[oo::input::MouseCode::Button5] = false;
	m_mousestate[oo::input::MouseCode::Button6] = false;
	m_mousestate[oo::input::MouseCode::Button7] = false;

	oo::EventManager::Subscribe<KeyLogging, ToolbarButtonEvent>(this, &KeyLogging::StartLogging);
	oo::EventManager::Subscribe<KeyLogging, ToolbarButtonEvent>(this, &KeyLogging::StopLogging);
	oo::EventManager::Subscribe<KeyLogging, ToolbarButtonEvent>(this, &KeyLogging::ToggleSimulate);
	oo::EventManager::Subscribe<KeyLogging, ToolbarButtonEvent>(this, &KeyLogging::SetEnableKeyLogging);
}									   

KeyLogging::~KeyLogging()
{
	m_keystate.clear();
	m_actionDown.clear();
	m_actionUp.clear();
	m_mousePosition.clear();
}

void KeyLogging::Show()
{
	/*ImGui::Begin("KeyLog");
	if (ImGui::Button(m_start ? "Stop" : "Start"))
	{
		m_start = !m_start;
		if (m_start)
		{
			if(m_mode == true)
				Reset();
			else
			{
				oo::input::SetSimulation(true);
				m_actionDownCounter = 0;
				m_actionUpCounter = 0;
				m_mousepositionCounter = 0;
				m_mousePressedCounter = 0;
			}
		}
		m_elaspedTime = 0;
		m_simulatedTime = 0;
	}
	ImGui::SameLine();
	if (ImGui::Button(m_mode ? "Logging" : "Simulate"))
	{
		m_mode = !m_mode;
	}*/
	if (m_start)
	{
		if (m_mode)
		{
			LoggingKeys();
		}
		else
		{
			SimulateKeys();
		}
	}
	//if(oo::input::IsAnyMouseButtonHeld())
	//	ImGui::Text("Mouse Button Pressed");
	//ImGui::End();

}

void KeyLogging::LoggingKeys()
{
	m_elaspedTime += oo::timer::dt();
	auto held = oo::input::GetKeysHeld();
	auto released = oo::input::GetKeysReleased();
	std::vector<oo::input::KeyCode> held_down;
	for (auto key : held)
	{
		if (m_keystate[static_cast<size_t>(key)] == false)
		{
			m_keystate[static_cast<size_t>(key)] = true;
			held_down.push_back(key);
		}
	}
	if(held_down.empty() == false)
		m_actionDown.push_back(TimedInputs(m_elaspedTime, std::move(held_down)));

	std::vector<oo::input::KeyCode> released_button;
	for (auto key : released)
	{
		if (m_keystate[static_cast<size_t>(key)] == true)
		{
			m_keystate[static_cast<size_t>(key)] = false;
			released_button.push_back(key);
		}
	}
	if(released_button.empty() == false)
		m_actionUp.push_back(TimedInputs(m_elaspedTime, std::move(released_button)));
	auto mousepress = oo::input::GetMouseButtonsPressed();
	auto mouserelease = oo::input::GetMouseButtonsReleased();
	for (auto button : mousepress)
	{
		m_mousePressed.push_back(TimedMouseInputs(m_elaspedTime, button));
	}
	for (auto button : mouserelease)
	{
		m_mousePressed.push_back(TimedMouseInputs(m_elaspedTime, button));
	}


	m_timeAccumulator += oo::timer::dt();
	auto delta = oo::input::GetMouseDelta();
	if (delta.first + delta.second != 0)
	{
		auto pos = oo::input::GetMousePosition();
		MousePos mp;
		mp.x = static_cast<short>(pos.first);
		mp.y = static_cast<short>(pos.second);
		mp.dx = static_cast<short>(delta.first);
		mp.dy = static_cast<short>(delta.second);
		m_mousePosition.push_back(std::pair(m_timeAccumulator, std::move(mp)));
		m_timeAccumulator = 0;
	}
}

void KeyLogging::SimulateKeys()
{
	m_simulatedTime += oo::timer::dt();
	if (m_mousePosition.empty() == false && m_mousepositionCounter < m_mousePosition.size())
	{
		auto& mousedata = m_mousePosition[m_mousepositionCounter];
		if (m_simulatedTime > m_timeAccumulator + mousedata.first)
		{
			auto& data = mousedata.second;
			oo::input::SimulatedMousePosition(data.x, data.y, data.dx, data.dy);
			++m_mousepositionCounter;
			m_timeAccumulator += mousedata.first;
		}
	}
	else
	{
		m_start = false;
		oo::input::SetSimulation(false);
		return;
	}

	if (m_actionDown.size() > m_actionDownCounter && m_simulatedTime >= m_actionDown[m_actionDownCounter].first)
	{
		auto item = m_actionDown[m_actionDownCounter];
		++m_actionDownCounter;//increment after using value
		if (item.second.size())
		{
			for (auto timedInputs : item.second)
			{
				oo::input::SimulateKeyInput(timedInputs, true);
			}
		}
	}
	if (m_actionUp.size() > m_actionUpCounter)
	{
		if (m_simulatedTime >= m_actionUp[m_actionUpCounter].first)
		{
			auto item = m_actionUp[m_actionUpCounter];
			++m_actionUpCounter;//increment after using value
			if (item.second.size())
			{
				for (auto timedInputs : item.second)
				{
					oo::input::SimulateKeyInput(timedInputs, false);
				}
			}
		}
	}
	if (m_mousePressed.empty() == false && m_mousePressed.size() > m_mousePressedCounter)
	{

		if (m_simulatedTime >= m_mousePressed[m_mousePressedCounter].first)
		{
			auto& pressed = m_mousePressed[m_mousePressedCounter];
			++m_mousePressedCounter;
			m_mousestate[pressed.second] = !m_mousestate[pressed.second];
		}
		for (auto &state : m_mousestate)
		{
			if (state.second)
				oo::input::SimulatedMouseButton(state.first);
		}
	}

}

void KeyLogging::Reset()
{
	m_keystate.resize(256);
	for (auto iter = 0; iter < 256; ++iter)
	{
		m_keystate.push_back(false);
	}
	for (auto& state : m_mousestate)
	{
		state.second = false;
	}
	m_actionDown.clear();
	m_actionUp.clear();
	m_mousePosition.clear();
	m_mousePressed.clear();
}

void KeyLogging::StartLogging(ToolbarButtonEvent* ev)
{
	if (ev->m_buttonType == ToolbarButtonEvent::ToolbarButton::PLAY && m_enable)
	{
		//m_mode = true;
		m_start = true;

		if (m_mode == true)
			Reset();
		else
		{
			oo::input::SetSimulation(true);
			m_actionDownCounter = 0;
			m_actionUpCounter = 0;
			m_mousepositionCounter = 0;
			m_mousePressedCounter = 0;
		}

		m_elaspedTime = 0;
		m_simulatedTime = 0;
		m_timeAccumulator = 0;
	}
}

void KeyLogging::StopLogging(ToolbarButtonEvent* ev)
{
	if (ev->m_buttonType == ToolbarButtonEvent::ToolbarButton::STOP)
	{
		//m_mode = true;
		m_start = false;
		m_elaspedTime = 0;
		m_simulatedTime = 0;
		m_timeAccumulator = 0;
	}
}

void KeyLogging::ToggleSimulate(ToolbarButtonEvent* ev)
{
	if (ev->m_buttonType == ToolbarButtonEvent::ToolbarButton::SET_SIMULATION_MODE)
	{
		m_mode = !m_mode;
	}
}

void KeyLogging::SetEnableKeyLogging(ToolbarButtonEvent* ev)
{
	if (ev->m_buttonType == ToolbarButtonEvent::ToolbarButton::SET_RECORD_ONPLAY)
	{
		m_enable = !m_enable;
		if (m_enable)
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Pressing play starts logging/resimulating keystrokes");
		else
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Stop logging/resimulating keystrokes on play");

	}
}
