#ifndef TIME_H
#define TIME_H

#include <chrono>

#include "Singleton.h"

class Timer final : public Singleton<Timer>
{
public:

	virtual ~Timer() = default;

	Timer(const Timer& other) = delete;
	Timer(Timer&& other) noexcept = delete;
	Timer& operator=(const Timer& other) = delete;
	Timer& operator=(Timer&& other) noexcept = delete;

	void Update();

	float GetElapsedSec() const;
	float GetFPS() const;

private:

	friend class Singleton<Timer>;
	Timer();

private:

	float m_ElapsedSec;

	std::chrono::high_resolution_clock::time_point m_LastTime;

};

#endif // !TIME_H