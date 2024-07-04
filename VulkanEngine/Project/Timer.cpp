#include <thread>

#include "Timer.h"

void Timer::Update()
{
	const auto now{ std::chrono::high_resolution_clock::now() };
	m_ElapsedSec = std::chrono::duration<float>(now - m_LastTime).count();
	m_LastTime = now;
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
	: m_ElapsedSec{}
	, m_LastTime{ std::chrono::high_resolution_clock::now() }
{
}