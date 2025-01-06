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

	AddCubeMapCamera();


	return true;
}

void CameraManager::Update(double dt)
{
	if (!m_cameraList.empty() && m_curCamera != nullptr)
	{
		m_curCamera->Update(dt);
	}

	for (auto& camera : m_CubecameraList)
	{
		camera->SetPos(m_curCamera->GetPosition());
		camera->CubeMapUpdate(dt);
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
	m_cameraList.push_back(camera);
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


const std::vector<Camera*>& CameraManager::GetCubeCameras()
{
	return m_CubecameraList;
}

void CameraManager::AddCubeMapCamera()
{
	auto curpos = m_curCamera->GetPosition();
	DirectX::XMFLOAT3 forward[6] = {
		DirectX::XMFLOAT3(curpos.x + 1,curpos.y,curpos.z),	//+X
		DirectX::XMFLOAT3(curpos.x - 1,curpos.y,curpos.z),	//-X
		DirectX::XMFLOAT3(curpos.x,curpos.y + 1,curpos.z),	//+Y
		DirectX::XMFLOAT3(curpos.x,curpos.y - 1,curpos.z),	//-Y
		DirectX::XMFLOAT3(curpos.x,curpos.y,curpos.z + 1),	//+Z
		DirectX::XMFLOAT3(curpos.x,curpos.y,curpos.z - 1),	//-Z

	};

	DirectX::XMFLOAT3 up[6] = {
		DirectX::XMFLOAT3(0,1,0),	//+X
		DirectX::XMFLOAT3(0,1,0),	//-X
		DirectX::XMFLOAT3(0,1,0),	//+Y
		DirectX::XMFLOAT3(0,-1,0),	//-Y
		DirectX::XMFLOAT3(0,1,0),	//+Z
		DirectX::XMFLOAT3(0,1,0),	//-Z

	};

	DirectX::XMFLOAT3 right[6] = {
		DirectX::XMFLOAT3(0,0,1),	//+X
		DirectX::XMFLOAT3(0,0,-1),	//-X
		DirectX::XMFLOAT3(0,0,1),	//+Y
		DirectX::XMFLOAT3(0,0,1),	//-Y
		DirectX::XMFLOAT3(0,0,1),	//+Z
		DirectX::XMFLOAT3(0,0,-1),	//-Z
	};

	for (int i = 0; i < 6; i++)
	{
		Camera* camera = new Camera(forward[i], up[i], right[i]);
		camera->Initialize(m_ratio);
		m_CubecameraList.push_back(camera);
	}

	
}
