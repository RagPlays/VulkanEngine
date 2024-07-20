#include <iostream>
#include <thread>

#include "Timer.h"

void Timer::Update()
{
	const auto now{ std::chrono::high_resolution_clock::now() };
	m_ElapsedSec = std::chrono::duration<float>(now - m_LastTime).count();
	m_LastTime = now;

	UpdateFPSPrint();
}

void Timer::UpdateFPSPrint() const
{
	if (!m_PrintFPS) return;
	
	static int frameCount{};
	static float totalWaitTime{};
	static const float updatTickTime{ 1.f };

	++frameCount;
	totalWaitTime += m_ElapsedSec;

	if (totalWaitTime >= updatTickTime)
	{
		const int fpsAverage{ static_cast<int>(1.f / (totalWaitTime / frameCount)) };

		std::cout << "FPS: " << fpsAverage << "\n";

		totalWaitTime -= updatTickTime;
		frameCount = 0;
	}
}

float Timer::GetElapsedSec() const
{
	return m_ElapsedSec;
}

float Timer::GetFPS() const
{
	return 1.f / m_ElapsedSec;
}

// Private Functions //
Timer::Timer()
	: m_PrintFPS{ false }
	, m_ElapsedSec{}
	, m_LastTime{ std::chrono::high_resolution_clock::now() }
{
}