#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include "Ouroboros/Core/KeyCode.h"
class KeyLogging
{
public:
	KeyLogging();
	~KeyLogging();

	using TimedInputs = std::pair<float, std::vector<oo::input::KeyCode>>;
	void Show();
	void LoggingKeys();
	void SimulateKeys();
	void Reset();
private:
	float m_elaspedTime;
	float m_simulatedTime;

	unsigned m_actionDownCounter = 0;
	std::deque<TimedInputs> m_actionDown;
	unsigned m_actionUpCounter = 0;
	std::deque<TimedInputs> m_actionUp;

	std::unordered_map<oo::input::KeyCode ,WORD> m_keymapping;
	std::vector<bool> m_keystate;
	bool m_start = false;
	bool m_mode = false;//true -> logging mode
};