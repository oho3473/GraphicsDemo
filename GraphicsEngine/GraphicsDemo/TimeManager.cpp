#include "TimeManager.h"
#include <windows.h>

TimeManager::TimeManager() : m_preCount(0), m_curCount(0), m_prePause(0), m_curPause(0), m_pauseTime(0),
m_deltaTime(0), m_totalTime(0), m_secPerCount(0), m_framePerCount(0), m_isPause(false)
{

}

TimeManager::~TimeManager()
{

}

bool TimeManager::Initialize()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_curCount);
	m_preCount = m_curCount;

	__int64 countPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSec);
	m_secPerCount = 1 / static_cast<double> (countPerSec);

	return true;
}

void TimeManager::Update(double dt)
{

	if (m_isPause)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_curPause);

		m_pauseTime += (m_curPause - m_prePause) * m_secPerCount;

		return;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&m_curCount);

	m_deltaTime = static_cast<double>(m_curCount - m_preCount) * m_secPerCount;
	m_totalTime += m_deltaTime * 1000;

	static double time = 0;
	time += m_deltaTime;

	static int count = 0;

	if (time > 1)
	{
		time -= 1;

		m_framePerCount = count;
		count = 0;
	}

	count++;

	m_preCount = m_curCount;
}

bool TimeManager::Finalize()
{
	return true;
}

void TimeManager::Pause()
{
	m_isPause = true;
	m_preCount = m_curCount;
}

void TimeManager::UnPause()
{
	m_isPause = false;
	m_preCount = m_prePause;
	m_curCount = m_curPause;

	m_pauseTime = 0;
}

double TimeManager::FPS() const
{
	return static_cast<double>(m_framePerCount);
}

double TimeManager::DeltaTime() const
{
	return m_deltaTime;
}

double TimeManager::TotalTime() const
{
	return m_totalTime;
}
