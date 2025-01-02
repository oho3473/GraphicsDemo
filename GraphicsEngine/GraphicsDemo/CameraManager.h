#pragma once
#include <vector>

#include "IManager.h"
#include "Camera.h"

class CameraManager : public IManager 
{
public:
	CameraManager(double ratio);
	~CameraManager();

	virtual bool Initialize() override;
	virtual void Update(double dt) override;
	virtual bool Finalize() override;

	void AddCamera(Camera* camera);
	void AddCamera();
	void SetCurCamera(int num);
	void SetCurCameraPos(DirectX::XMFLOAT3 pos);
	DirectX::XMFLOAT4X4 WorldViewProj() const;
	

	DirectX::XMFLOAT4X4 View();
	DirectX::XMFLOAT4X4 Proj();
	void OnResize(double ratio);
	DirectX::SimpleMath::Vector3 GetCamerPos() const;


private:
	Camera* m_curCamera;
	std::vector<Camera*> m_cameraList;
	double m_ratio;
};

