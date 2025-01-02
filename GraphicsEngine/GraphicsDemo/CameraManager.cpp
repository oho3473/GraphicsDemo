#include "CameraManager.h"

CameraManager::CameraManager(double ratio) : m_curCamera(nullptr),m_ratio(ratio)
{

}

CameraManager::~CameraManager()
{
	for (auto camera : m_cameraList)
	{
		delete camera;
	}
}

bool CameraManager::Initialize()
{
	//main camera
	AddCamera();

	return true;
}

void CameraManager::Update(double dt)
{
	if (!m_cameraList.empty() && m_curCamera != nullptr)
	{
		m_curCamera->Update(dt);
	}
}

bool CameraManager::Finalize()
{
	for (auto camera : m_cameraList)
	{
		delete camera;
	}

	m_cameraList.clear();

	return true;
}

void CameraManager::AddCamera(Camera* camera)
{
	m_cameraList.push_back(m_curCamera);
}

void CameraManager::AddCamera()
{
	m_cameraList.push_back(new Camera());
	m_cameraList.back()->Initialize(m_ratio);
	m_curCamera = m_cameraList.back();
}

void CameraManager::SetCurCamera(int num)
{
	if (num < 0)
	{
		return;
	}

	m_curCamera = m_cameraList[num];
}

void CameraManager::SetCurCameraPos(DirectX::XMFLOAT3 pos)
{
	m_curCamera->SetPos(pos);
}

DirectX::XMFLOAT4X4 CameraManager::WorldViewProj() const
{
	return m_curCamera->WorldViewProj();
}


DirectX::XMFLOAT4X4 CameraManager::View()
{
	return m_curCamera->View();

}

DirectX::XMFLOAT4X4 CameraManager::Proj()
{
	return m_curCamera->Proj();

}

void CameraManager::OnResize(double ratio)
{
	m_ratio = ratio;
	m_curCamera->OnResize(m_ratio);
}

DirectX::SimpleMath::Vector3 CameraManager::GetCamerPos() const
{
	return m_curCamera->GetPosition();
}
