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

#include "App/Editor/Utility/FileSystemUtills.h"

#include <App/Editor/Utility/Windows_Utill.h>

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <fstream>

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
	if (m_enable)
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
		if (ImGui::Begin("Key Simulate", &m_enable,ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration))
		{
			if (ImGui::Button("Save File"))
			{
				WindowsUtilities::FileDialogue_Folder([this](const std::filesystem::path& p) {this->SaveKeylogs(p); });
			}
			if (ImGui::Button("Load File"))
			{
				WindowsUtilities::FileDialogue_Generic(L"Replays", L"*.OOReplays", [this](const std::filesystem::path& p) {this->LoadKeylogs(p); });
			}
			ImGui::Text("MousePositions: %u MouseClicks: %u", m_mousePosition.size(), m_mousePressed.size());
			ImGui::Text("ActionDown: %u ActionUp: %u", m_actionDown.size(), m_actionUp.size());
			ImGui::End();
		}
		else
			ImGui::End();
		ImGui::PopStyleColor();
	}
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

void KeyLogging::SaveKeylogs(const std::filesystem::path& path)
{
	rapidjson::Document doc;
	rapidjson::Value& val = doc.SetObject();

	rapidjson::Value actionDown(rapidjson::kObjectType);
	rapidjson::Value actionUp(rapidjson::kObjectType);
	rapidjson::Value mousePosition(rapidjson::kObjectType);
	rapidjson::Value mousePressed(rapidjson::kObjectType);

	for (const auto& item : m_actionDown)
	{
		rapidjson::Value arrVal(rapidjson::kArrayType);
		rapidjson::Value arrVal2(rapidjson::kArrayType);
		arrVal.PushBack(item.first, doc.GetAllocator());
		for (auto kc : item.second)
		{
			int k = (int)kc;
			arrVal2.PushBack(k, doc.GetAllocator());
		}
		arrVal.PushBack(arrVal2, doc.GetAllocator());
		actionDown.AddMember("", arrVal, doc.GetAllocator());
	}
	for (const auto& item : m_actionUp)
	{
		rapidjson::Value arrVal(rapidjson::kArrayType);
		rapidjson::Value arrVal2(rapidjson::kArrayType);
		arrVal.PushBack(item.first, doc.GetAllocator());
		for (auto kc : item.second)
		{
			int k = (int)kc;
			arrVal2.PushBack(k, doc.GetAllocator());
		}
		arrVal.PushBack(arrVal2, doc.GetAllocator());
		actionUp.AddMember("", arrVal, doc.GetAllocator());
	}
	for (const auto& item : m_mousePosition)
	{
		rapidjson::Value arrVal(rapidjson::kArrayType);
		arrVal.PushBack(item.first, doc.GetAllocator());
		arrVal.PushBack(item.second.dx, doc.GetAllocator());
		arrVal.PushBack(item.second.dy, doc.GetAllocator());
		arrVal.PushBack(item.second.x, doc.GetAllocator());
		arrVal.PushBack(item.second.y, doc.GetAllocator());
		mousePosition.AddMember("", arrVal, doc.GetAllocator());
	}
	for (const auto& item : m_mousePressed)
	{
		rapidjson::Value arrVal(rapidjson::kArrayType);
		arrVal.PushBack(item.first, doc.GetAllocator());
		int k = (int)item.second;
		arrVal.PushBack(k, doc.GetAllocator());
		mousePressed.AddMember("", arrVal, doc.GetAllocator());
	}
	val.AddMember("down", actionDown, doc.GetAllocator());
	val.AddMember("up", actionUp, doc.GetAllocator());
	val.AddMember("mousepos", mousePosition, doc.GetAllocator());
	val.AddMember("mousepress", mousePressed, doc.GetAllocator());

	auto p = FileSystemUtils::CreateItem((path / "Replay").string(), ".OOReplays");
	std::ofstream ofs(p, std::fstream::out | std::fstream::trunc);
	if (ofs.good())
	{
		rapidjson::OStreamWrapper osw(ofs);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);
		writer.SetMaxDecimalPlaces(6);
		doc.Accept(writer);
		ofs.close();
	}
}

void KeyLogging::LoadKeylogs(const std::filesystem::path& path)
{
	std::ifstream ifs(path);
	if (ifs.peek() == std::ifstream::traits_type::eof())
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "Scene File is not valid!");
		return;
	}
	Reset();//reset
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	
	rapidjson::Value& down = doc.FindMember("down")->value;
	rapidjson::Value& up = doc.FindMember("up")->value;
	rapidjson::Value& mousepos = doc.FindMember("mousepos")->value;
	rapidjson::Value& mousepress = doc.FindMember("mousepress")->value;

	for (auto iter = down.MemberBegin(); iter != down.MemberEnd(); ++iter)
	{
		auto arr = iter->value.GetArray();
		float t = arr[0].GetFloat();
		auto keycommands = arr[1].GetArray();
		std::vector<oo::input::KeyCode> key;
		for (auto i = 0; i < keycommands.Size(); ++i)
		{
			key.push_back((oo::input::KeyCode)keycommands[0].GetInt());
		}
		m_actionDown.push_back(std::pair(t, key));
	}
	for (auto iter = up.MemberBegin(); iter != up.MemberEnd(); ++iter)
	{
		auto arr = iter->value.GetArray();
		float t = arr[0].GetFloat();
		auto keycommands = arr[1].GetArray();
		std::vector<oo::input::KeyCode> key;
		for (auto i = 0; i < keycommands.Size(); ++i)
		{
			key.push_back((oo::input::KeyCode)keycommands[0].GetInt());
		}
		m_actionUp.push_back(std::pair(t, key));
	}

	for (auto iter = mousepos.MemberBegin(); iter != mousepos.MemberEnd(); ++iter)
	{
		auto arr = iter->value.GetArray();
		float t = arr[0].GetFloat();
		MousePos mp;
		mp.dx = arr[1].GetInt();
		mp.dy = arr[2].GetInt();
		mp.x = arr[3].GetInt();
		mp.y = arr[4].GetInt();

		m_mousePosition.push_back(std::pair(t, mp));
	}

	for (auto iter = mousepress.MemberBegin(); iter != mousepress.MemberEnd(); ++iter)
	{
		auto arr = iter->value.GetArray();
		float t = arr[0].GetFloat();
		oo::input::MouseCode key;
		key = (oo::input::MouseCode)arr[1].GetInt();
		m_mousePressed.push_back(std::pair(t, key));
	}
	ifs.close();
}

