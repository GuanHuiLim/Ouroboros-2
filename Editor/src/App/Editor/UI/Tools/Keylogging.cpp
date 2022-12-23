#include "pch.h"
#include "Keylogging.h"
#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Core/Timer.h"
#include <WinUser.h>
#include <imgui/imgui.h>
KeyLogging::KeyLogging()
{
	Reset();
	m_keymapping[oo::input::KeyCode::A] = 0x41;
	m_keymapping[oo::input::KeyCode::B] = 0x42;
	m_keymapping[oo::input::KeyCode::C] = 0x43;
	m_keymapping[oo::input::KeyCode::D] = 0x44;
	m_keymapping[oo::input::KeyCode::E] = 0x45;
	m_keymapping[oo::input::KeyCode::F] = 0x46;
	m_keymapping[oo::input::KeyCode::G] = 0x47;
	m_keymapping[oo::input::KeyCode::H] = 0x48;
	m_keymapping[oo::input::KeyCode::I] = 0x49;
	m_keymapping[oo::input::KeyCode::J] = 0x4A;
	m_keymapping[oo::input::KeyCode::K] = 0x4B;
	m_keymapping[oo::input::KeyCode::L] = 0x4C;
	m_keymapping[oo::input::KeyCode::M] = 0x4D;
	m_keymapping[oo::input::KeyCode::N] = 0x4E;
	m_keymapping[oo::input::KeyCode::O] = 0x4F;
	m_keymapping[oo::input::KeyCode::P] = 0x50;
	m_keymapping[oo::input::KeyCode::Q] = 0x51;
	m_keymapping[oo::input::KeyCode::R] = 0x52;
	m_keymapping[oo::input::KeyCode::S] = 0x53;
	m_keymapping[oo::input::KeyCode::T] = 0x54;
	m_keymapping[oo::input::KeyCode::U] = 0x55;
	m_keymapping[oo::input::KeyCode::V] = 0x56;
	m_keymapping[oo::input::KeyCode::W] = 0x57;
	m_keymapping[oo::input::KeyCode::X] = 0x58;
	m_keymapping[oo::input::KeyCode::Y] = 0x59;
	m_keymapping[oo::input::KeyCode::Z] = 0x5A;
	m_keymapping[oo::input::KeyCode::LSHIFT] = VK_LSHIFT;
	m_keymapping[oo::input::KeyCode::RSHIFT] = VK_RSHIFT;
	m_keymapping[oo::input::KeyCode::TAB] = VK_TAB;
	m_keymapping[oo::input::KeyCode::LCTRL] = VK_LCONTROL;
	m_keymapping[oo::input::KeyCode::RCTRL] = VK_RCONTROL;
	m_keymapping[oo::input::KeyCode::LALT] = VK_LMENU;
	m_keymapping[oo::input::KeyCode::RALT] = VK_RMENU;
	m_keymapping[oo::input::KeyCode::SPACE] = VK_SPACE;
	m_keymapping[oo::input::KeyCode::ENTER] = VK_RETURN;
}

KeyLogging::~KeyLogging()
{
	m_keystate.clear();
	m_actionDown.clear();
	m_actionUp.clear();
}

void KeyLogging::Show()
{
	ImGui::Begin("KeyLog");
	if (ImGui::Button("Test"))
	{
		INPUT inputs[4] = {};
		ZeroMemory(inputs, sizeof(inputs));

		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = VK_LWIN;

		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.wVk = 0x44;

		inputs[2].type = INPUT_KEYBOARD;
		inputs[2].ki.wVk = 0x44;
		inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

		inputs[3].type = INPUT_KEYBOARD;
		inputs[3].ki.wVk = VK_LWIN;
		inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

		UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	}
	if (ImGui::Button(m_start ? "Stop" : "Start"))
	{
		m_start = !m_start;
		if (m_start)
		{
			if(m_mode == true)
				Reset();
			else
			{
				m_actionDownCounter = 0;
				m_actionUpCounter = 0;
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
	ImGui::End();
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
}

void KeyLogging::SimulateKeys()
{
	m_simulatedTime += oo::timer::dt();
	if (m_actionDown.size() > m_actionDownCounter && m_simulatedTime >= m_actionDown[m_actionDownCounter].first)
	{
		auto item = m_actionDown[m_actionDownCounter];
		++m_actionDownCounter;//increment after using value
		if (item.second.size())
		{
			INPUT* input = new INPUT[item.second.size()];
			ZeroMemory(input, sizeof(INPUT) * item.second.size());
			int counter = 0;
			for (auto timedInputs : item.second)
			{
				input[counter].type = INPUT_KEYBOARD;
				input[counter].ki.wVk = m_keymapping[timedInputs];
				//input[counter].ki.dwFlags = KEYEVENTF_KEYUP;
				++counter;
			}
			
			SendInput(item.second.size(), input, sizeof(INPUT));
			delete[] input;
		}
	}
	if (m_actionUp.size() > m_actionUpCounter && m_simulatedTime >= m_actionUp[m_actionUpCounter].first)
	{
		auto item = m_actionUp[m_actionUpCounter];
		++m_actionUpCounter;//increment after using value
		if (item.second.size())
		{
			INPUT* input = new INPUT[item.second.size()];
			ZeroMemory(input, sizeof(INPUT) * item.second.size());
			int counter = 0;
			for (auto timedInputs : item.second)
			{
				input[counter].type = INPUT_KEYBOARD;
				input[counter].ki.wVk = m_keymapping[timedInputs];
				input[counter].ki.dwFlags = KEYEVENTF_KEYUP;
				++counter;
			}
			SendInput(item.second.size(), input, sizeof(INPUT));
			delete[] input;
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
	m_actionDown.clear();
	m_actionUp.clear();
}
