#pragma once


/// <summary>
/// �Ŵ��� �� �������̽� Ŭ����
/// </summary>
class IManager
{
public:
	virtual bool Initialize() abstract;
	virtual void Update(double dt) abstract;
	virtual bool Finalize() abstract;

private:
};

