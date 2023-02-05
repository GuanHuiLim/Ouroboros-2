#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include <filesystem>
#include "Ouroboros/Core/KeyCode.h"
#include "Ouroboros/Core/MouseCode.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"
class KeyLogging
{
public:
	KeyLogging();
	~KeyLogging();

	using TimedInputs = std::pair<float, std::vector<oo::input::KeyCode>>;
	using TimedMouseInputs = std::pair<float, oo::input::MouseCode>;
	void Show();
	void LoggingKeys();
	void SimulateKeys();
	void Reset();

	void StartLogging(ToolbarButtonEvent* ev);
	void StopLogging(ToolbarButtonEvent* ev);
	void ToggleSimulate(ToolbarButtonEvent* ev);
	void SetEnableKeyLogging(ToolbarButtonEvent* ev);
	static bool GetMode() { return m_mode; };
	static bool GetEnable() { return m_enable; };
public:
	void SaveKeylogs(const std::filesystem::path& path);
	void LoadKeylogs(const std::filesystem::path& path);
private:
	float m_elaspedTime;
	float m_simulatedTime;

	unsigned m_actionDownCounter = 0;
	std::vector<TimedInputs> m_actionDown;
	unsigned m_actionUpCounter = 0;
	std::vector<TimedInputs> m_actionUp;

	struct MousePos
	{
		short x;
		short y;
		short dx;
		short dy;
	};
	float m_timeAccumulator = 0.0f;
	const float m_granularity = 0.016f;
	unsigned m_mousepositionCounter = 0;
	std::vector < std::pair<float, MousePos> > m_mousePosition;
	unsigned m_mousePressedCounter = 0;
	std::vector<TimedMouseInputs> m_mousePressed;

	std::vector<bool> m_keystate;
	std::unordered_map<oo::input::MouseCode, bool> m_mousestate;
	bool m_start = false;
	inline static bool m_mode = false;//true -> logging mode
	inline static bool m_enable = false;
};