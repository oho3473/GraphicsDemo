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
		//camera->SetPos(m_curCamera->GetPosition());
		auto curpos = m_curCamera->GetPosition();

		DirectX::XMFLOAT3 forward[6] = {
		DirectX::XMFLOAT3(1, 0, 0),  // +X
		DirectX::XMFLOAT3(-1, 0, 0), // -X
		DirectX::XMFLOAT3(0, 1, 0),  // +Y
		DirectX::XMFLOAT3(0, -1, 0), // -Y
		DirectX::XMFLOAT3(0, 0, 1),  // +Z
		DirectX::XMFLOAT3(0, 0, -1)  // -Z
		};

		DirectX::XMFLOAT3 up[6] = {
			DirectX::XMFLOAT3(0, 1, 0),  // +X
			DirectX::XMFLOAT3(0, 1, 0),  // -X
			DirectX::XMFLOAT3(0, 0, -1), // +Y
			DirectX::XMFLOAT3(0, 0, 1),  // -Y
			DirectX::XMFLOAT3(0, 1, 0),  // +Z
			DirectX::XMFLOAT3(0, 1, 0)   // -Z
		};

		DirectX::XMMATRIX viewMatrices[6];
		DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&curpos); // 큐브 맵의 중심

		for (int i = 0; i < 6; ++i) {
			DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(eyePos, DirectX::XMLoadFloat3(&forward[i]));
			DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up[i]);

			viewMatrices[i] = DirectX::XMMatrixLookAtLH(eyePos, lookAt, upVec);
			camera->CubeMapUpdate(dt,viewMatrices[i]);
		}


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
	DirectX::XMFLOAT3(1, 0, 0),  // +X
	DirectX::XMFLOAT3(-1, 0, 0), // -X
	DirectX::XMFLOAT3(0, 1, 0),  // +Y
	DirectX::XMFLOAT3(0, -1, 0), // -Y
	DirectX::XMFLOAT3(0, 0, 1),  // +Z
	DirectX::XMFLOAT3(0, 0, -1)  // -Z
	};

	DirectX::XMFLOAT3 up[6] = {
		DirectX::XMFLOAT3(0, 1, 0),  // +X
		DirectX::XMFLOAT3(0, 1, 0),  // -X
		DirectX::XMFLOAT3(0, 0, -1), // +Y
		DirectX::XMFLOAT3(0, 0, 1),  // -Y
		DirectX::XMFLOAT3(0, 1, 0),  // +Z
		DirectX::XMFLOAT3(0, 1, 0)   // -Z
	};

	DirectX::XMMATRIX viewMatrices[6];
	DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&curpos); // 큐브 맵의 중심

	for (int i = 0; i < 6; ++i) 
	{


		Camera* camera = new Camera();
		camera->Initialize(m_ratio);
		m_CubecameraList.push_back(camera);
	}
}
