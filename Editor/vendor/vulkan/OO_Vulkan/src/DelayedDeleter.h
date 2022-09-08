#pragma once
#include <functional>
#include <array>
#include <vector>

class DelayedDeleter
{
public:

	static DelayedDeleter* get();
	static void Shutdown();

	void DeleteAfterFrames(std::function<void()> fn, size_t frames = 3);
	void DeleteAfterSeconds(std::function<void()> fn, float seconds = 3);
	void Update(float deltaTime = 0.0f);
	void Clear();

private:
	static constexpr int MAX_FRAMES = 3;
	size_t m_frame{ 0 };
	std::array<std::vector<std::function<void()>>, MAX_FRAMES> m_funcs;
	

	inline static DelayedDeleter* s_deleter{nullptr};
};

