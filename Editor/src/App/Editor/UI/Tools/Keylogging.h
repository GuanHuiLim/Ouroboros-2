#pragma once
#include <unordered_map>
#include <vector>
#include <deque>
#include "Ouroboros/Core/KeyCode.h"
#include "Ouroboros/Core/MouseCode.h"
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
private:
	float m_elaspedTime;
	float m_simulatedTime;

	unsigned m_actionDownCounter = 0;
	std::deque<TimedInputs> m_actionDown;
	unsigned m_actionUpCounter = 0;
	std::deque<TimedInputs> m_actionUp;
	union MousePos
	{
		short x;
		short y;
		int data;
	};
	float m_timeAccumulator = 0.0f;
	const float m_granularity = 0.016;
	unsigned m_mousepositionCounter = 0;
	std::deque<MousePos> m_mousePosition;
	unsigned m_mousePressedCounter = 0;
	std::deque<TimedMouseInputs> m_mousePressed;

	std::vector<bool> m_keystate;
	std::unordered_map<oo::input::MouseCode, bool> m_mousestate;
	bool m_start = false;
	bool m_mode = false;//true -> logging mode
};