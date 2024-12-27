#pragma once
#include "IManager.h"

class TimeManager : public IManager
{
public:
	TimeManager();
	~TimeManager();

	virtual bool Initialize() override;
	virtual void Update(double dt) override;
	virtual bool Finalize() override;

	void Pause();
	void UnPause();

	double FPS() const;
	double DeltaTime() const;
	double TotalTime() const;

private:

	__int64 m_preCount; //���� ����
	__int64 m_curCount; //���� ����
	__int64 m_prePause; //���� ����
	__int64 m_curPause; //���� ����

	double m_pauseTime; //���� �ð�
	double m_deltaTime;
	double m_totalTime;

	double m_secPerCount; //���� �� �ð�
	__int64 m_framePerCount; // �ʴ� ����

	bool m_isPause;
};

