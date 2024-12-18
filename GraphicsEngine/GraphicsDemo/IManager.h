#pragma once


/// <summary>
/// 매니저 급 인터페이스 클래스
/// </summary>
class IManager
{
public:
	virtual bool Initialize() abstract;
	virtual void Update(double dt) abstract;
	virtual bool Finalize() abstract;

private:
};

