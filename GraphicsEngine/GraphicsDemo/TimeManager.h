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

	__int64 m_preCount; //檜瞪 偎熱
	__int64 m_curCount; //⑷營 偎熱
	__int64 m_prePause; //⑷營 偎熱
	__int64 m_curPause; //⑷營 偎熱

	double m_pauseTime; //誇轔 衛除
	double m_deltaTime;
	double m_totalTime;

	double m_secPerCount; //偃熱 渡 衛除
	__int64 m_framePerCount; // 蟾渡 偎熱

	bool m_isPause;
};

