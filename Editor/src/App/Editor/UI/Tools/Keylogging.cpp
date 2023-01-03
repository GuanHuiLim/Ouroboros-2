#include "pch.h"
#include "Keylogging.h"
#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Core/Timer.h"
#include "Ouroboros/Core/Application.h"
#include <WinUser.h>
#include <imgui/imgui.h>
#include <sdl2/SDL.h>

#define DEBUG_KEYBOARD
KeyLogging::KeyLogging()
{
	Reset();
	m_mousestate[oo::input::MouseCode::Button0] = false;
	m_mousestate[oo::input::MouseCode::Button1] = false;
	m_mousestate[oo::input::MouseCode::Button2] = false;
	m_mousestate[oo::input::MouseCode::Button3] = false;
	m_mousestate[oo::input::MouseCode::Button4] = false;
	m_mousestate[oo::input::MouseCode::Button5] = false;
	m_mousestate[oo::input::MouseCode::Button6] = false;
	m_mousestate[oo::input::MouseCode::Button7] = false;
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
	ImGui::Begin("KeyLog");
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
	}
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
	if(oo::input::IsAnyMouseButtonHeld())
		ImGui::Text("Mouse Button Pressed");
	ImGui::End();

}

void KeyLogging::LoggingKeys()
{
	m_timeAccumulator += oo::timer::dt();
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

	if (m_timeAccumulator > m_granularity)
	{
		m_timeAccumulator = 0;
		auto pos = oo::input::GetMousePosition();
		MousePos mp;
		mp.x = static_cast<short>(pos.first);
		mp.y = static_cast<short>(pos.second);
		m_mousePosition.push_back(std::move(mp));
	}
}

void KeyLogging::SimulateKeys()
{
	m_simulatedTime += oo::timer::dt();
	m_timeAccumulator += oo::timer::dt();
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
	if (m_mousePosition.empty() == false && m_mousepositionCounter < m_mousePosition.size())
	{
		if (m_timeAccumulator > m_granularity)
		{
			m_timeAccumulator = 0;
			auto& data = m_mousePosition[m_mousepositionCounter];
			oo::input::SimulatedMousePosition(data.x, data.y);
			++m_mousepositionCounter;
		}
	}
	else
	{
		m_start = false;
		oo::input::SetSimulation(false);
		return;
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
