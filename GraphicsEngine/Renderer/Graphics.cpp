#include "Graphics.h"
#include "Device.h"

#include "ResourceManager.h"
#include "ModelLoader.h"
#include "LightManager.h"
#include "Animator.h"
#include "PassManager.h"
#include "DebugDrawManager.h"
#include "UIManager.h"

#include "ModelData.h"
#include "Mesh.h"
#include "Animation.h"


Graphics::Graphics(HWND hWnd) : m_hWnd(hWnd)
, m_wndSize()
, m_Device(std::make_shared<Device>())
, m_ResourceManager(std::make_shared<ResourceManager>())
, m_Loader(std::make_shared <ModelLoader>())
, m_LightManager(std::make_shared<LightManager>())
, m_Animator(std::make_shared <Animator>())
//, m_DecalManager(std::make_shared <DecalManager>())
, m_DebugDrawManager(std::make_shared <DebugDrawManager>())
, m_UIManager(std::make_shared <UIManager>())
, m_PassManager(std::make_shared <PassManager>()), IGraphics()
{

}

Graphics::~Graphics()
{

}

bool Graphics::Initialize()
{
	//srv 생성을 위한 com 초기화
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	GetClientRect(m_hWnd, &m_wndSize);

	m_Device->Initialize(m_hWnd);
	m_ResourceManager->Initialize(m_Device);
	m_Loader->Initialize(m_ResourceManager, m_Device);
	m_LightManager->Initialize(m_ResourceManager);
	m_Animator->Initialize(m_ResourceManager);
	m_DebugDrawManager->Initialize(m_Device, m_ResourceManager);
	m_UIManager->Initialize(m_Device, m_ResourceManager);
	m_PassManager->Initialize(m_Device, m_ResourceManager,m_DebugDrawManager, m_LightManager,m_UIManager);

	m_CurViewPort = m_ResourceManager->Create<ViewPort>(L"Main", m_wndSize).lock();

	OnResize(m_hWnd, false);

	return true;
}

void Graphics::CulingUpdate()
{
	for (auto& data : m_RenderVector)
	{
		m_AfterCulling.push_back(data);
	}

	//Culling();

}

void Graphics::AnimationUpdate(double dt)
{
	m_Animator->Update(m_AfterCulling);

}

void Graphics::Update(double dt)
{
	for (auto& data : m_RenderVector)
	{
		m_AfterCulling.push_back(data);
	}
	m_Animator->Update(m_AfterCulling);

	m_PassManager->Update(m_AfterCulling);
	m_LightManager->Update(m_Lights);

	m_AfterCulling.clear();
}

void Graphics::EndUpdate(double dt)
{
	m_AfterCulling.clear();

}

bool Graphics::Finalize()
{
	if (!m_AfterCulling.empty())
	{
		m_AfterCulling.clear();
	}

	if (!m_RenderVector.empty())
	{
		m_RenderVector.clear();
	}

	m_CurViewPort.reset();
	m_Device.reset();

	m_Loader.reset();
	m_Animator.reset();
	m_PassManager.reset();

	m_ResourceManager.reset();
	m_LightManager.reset();
	m_UIManager.reset();

	return true;

}

void Graphics::BeginRender()
{
	FLOAT Black[4] = { 0.f,0.f,0.f,1.f };
	const DirectX::SimpleMath::Color white = { 1.f, 1.f, 1.f, 1.f };
	const DirectX::SimpleMath::Color red = { 1.f, 0.f, 0.f, 1.f };
	const DirectX::SimpleMath::Color green = { 0.f, 1.f, 0.f, 1.f };
	const DirectX::SimpleMath::Color blue = { 0.f, 0.f, 1.f, 1.f };
	const DirectX::SimpleMath::Color gray = { 0.15f, 0.15f, 0.15f, 1.f };
	const DirectX::SimpleMath::Color black = { 0.f, 0.f, 0.f, 1.f };

	for (int i = 0; i < m_RTVs.size(); i++)
	{
		if (i == 0)
		{
			m_Device->BeginRender(m_RTVs[i].lock()->Get(), m_DSVs[0].lock()->Get(), blue);
		}
		else
		{
			m_Device->BeginRender(m_RTVs[i].lock()->Get(), m_DSVs[1].lock()->Get(), gray);
		}
	}

	for (int i = 0; i < m_PBRRTVs.size(); i++)
	{
		m_Device->Context()->ClearRenderTargetView(m_PBRRTVs[i].lock()->Get(), gray);
	}
}

void Graphics::Render(float deltaTime)
{
	m_PassManager->Render(deltaTime);

}

void Graphics::EndRender()
{
	m_Device->EndRender();

}

void Graphics::OnResize(HWND hwnd, bool isFullScreen)
{
	m_hWnd = hwnd;
	GetClientRect(m_hWnd, &m_wndSize);

	m_PBRRTVs.clear();
	m_RTVs.clear();
	m_DSVs.clear();

	m_ResourceManager->OnResize(m_wndSize, isFullScreen);
	m_CurViewPort = m_ResourceManager->Create<ViewPort>(L"Main", m_wndSize).lock();


	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"RTV_Main"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"GBuffer"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Albedo"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Normal"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Position"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Depth"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Metalic"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"LightMap"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Roughness"));
	m_RTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Emissive"));

	m_DSVs.push_back(m_ResourceManager->Get<DepthStencilView>(L"DSV_Main"));
	m_DSVs.push_back(m_ResourceManager->Get<DepthStencilView>(L"DSV_Deferred"));


	m_PBRRTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Fresnel"));
	m_PBRRTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Distribute"));
	m_PBRRTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"Geometry"));
	m_PBRRTVs.push_back(m_ResourceManager->Get<RenderTargetView>(L"NdotL"));

	m_PassManager->OnResize();


	m_Device->Context()->OMSetRenderTargets(1, m_RTVs[0].lock()->GetAddress(), m_DSVs[0].lock()->Get());
	m_Device->Context()->RSSetViewports(1, m_CurViewPort->Get());
}

void Graphics::DebugRenderONOFF(bool isRender)
{
	m_PassManager->SetDebugDraw(isRender);
}

void Graphics::EraseObject(uint32_t EntityID)
{
	auto find = FindEntity(EntityID);
	if (find != m_RenderVector.end())
	{
		m_RenderVector.erase(find);
	}
}

void Graphics::UpdateModel(uint32_t EntityID)
{

}

bool Graphics::AddRenderModel(std::shared_ptr<RenderData> data)
{
	auto find = FindEntity(data->EntityID);
	if (find != m_RenderVector.end())
	{


		(*find) = data;
	}
	else
	{
		if (data->isSkinned)
		{
			std::wstring id = std::to_wstring(data->EntityID);

			if (m_ResourceManager->Get<ConstantBuffer<MatrixPallete>>(id).lock() == nullptr)
			{
				m_ResourceManager->Create<ConstantBuffer<MatrixPallete>>(id, ConstantBufferType::Default);
			}
		}

		m_RenderVector.push_back(data);
	}

	return true;
}

void Graphics::SetCamera(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, const DirectX::SimpleMath::Matrix& orthoProj)
{
	m_View = view;
	m_Proj = proj;
	m_ViewProj = view * proj;

	DirectX::SimpleMath::Matrix  cb_worldviewproj;
	DirectX::SimpleMath::Matrix cb_view;
	DirectX::SimpleMath::Matrix cb_proj;
	DirectX::SimpleMath::Matrix cb_viewInverse;
	DirectX::SimpleMath::Matrix cb_projInverse;
	cb_worldviewproj = m_ViewProj;

	//상수 버퍼는 계산 순서때문에 전치한다
	cb_worldviewproj = m_ViewProj.Transpose();
	cb_view = m_View.Transpose();
	cb_proj = m_Proj.Transpose();

	DirectX::SimpleMath::Matrix viewInverse = view.Invert();
	cb_viewInverse = viewInverse.Transpose();
	DirectX::SimpleMath::Matrix projInverse = proj.Invert();
	cb_projInverse = projInverse.Transpose();


	///testCulling 쓸때는 주석 필요
	{
		//절두체
		DirectX::BoundingFrustum::CreateFromMatrix(m_Frustum, m_Proj);

		//회전이 왜 반대로 먹음..? -> view 자체가 카메라의 기준의 세상을 표현한 행렬
		//우리가 frustum을 구성하려면 카메라 자체의 위치와 회전 값이 필요함
		//view == camera invert , 우린 camera 자체가 필요함 즉 view invert를 써야함


		m_Frustum.Orientation = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(viewInverse);

		//카메라위치
		m_Frustum.Origin = { viewInverse._41,viewInverse._42,viewInverse._43 };
	}

	std::weak_ptr<ConstantBuffer<CameraData>> Camera = m_ResourceManager->Get<ConstantBuffer<CameraData>>(L"Camera");
	Camera.lock()->m_struct.view = cb_view;
	Camera.lock()->m_struct.proj = cb_proj;
	Camera.lock()->m_struct.viewInverse = cb_viewInverse;
	Camera.lock()->m_struct.projInverse = cb_projInverse;
	Camera.lock()->m_struct.worldviewproj = cb_worldviewproj;
	Camera.lock()->m_struct.orthoProj = orthoProj;
	Camera.lock()->Update();
}

void Graphics::AddLight(uint32_t EntityID, LightType kind, LightData data)
{
	if (m_Lights.find(EntityID) == m_Lights.end())
	{
		m_Lights.insert(std::pair<uint32_t, LightData>(EntityID, data));
	}
}

void Graphics::EraseLight(uint32_t EntityID, LightType kind)
{
	if (m_Lights.find(EntityID) != m_Lights.end())
	{
		m_Lights.erase(EntityID);
		EraseObject(EntityID);
		m_LightManager->EraseData(EntityID, kind);
	}
}

void Graphics::UpdateLightData(uint32_t EntityID, LightType kind, LightData data)
{
	if (m_Lights.find(EntityID) != m_Lights.end())
	{
		m_Lights[EntityID] = data;
	}
}

const double Graphics::GetDuration(std::wstring name, int index)
{
	std::shared_ptr<ModelData> curAni = m_ResourceManager->Get<ModelData>(name).lock();
	if (curAni != nullptr)
	{
		if (!curAni->m_Animations.empty() && 0 <= index && index < curAni->m_Animations.size())
		{
			//전체 tick / 초당 틱 == 애니메이션 재생시간
			return curAni->m_Animations[index]->m_Duration / curAni->m_Animations[index]->m_TickFrame;
		}
	}


	return 0;
}

void Graphics::SetCubeCamera(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix pro, int index)
{
	if (index >= 0 && index < 6)
	{
		m_CubeMapCameras[index].proj = pro;
		m_CubeMapCameras[index].view = view;
		m_CubeMapCameras[index].viewInverse = view.Invert();
	}
}

void Graphics::ChangeDebugQuad(const debug::quadstate state)
{
	m_PassManager->ChageDebugQuad(state);
}

void Graphics::DrawSphere(const debug::SphereInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawAABB(const debug::AABBInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawOBB(const debug::OBBInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawFrustum(const debug::FrustumInfo& info)
{
	m_DebugDrawManager->AddTask(info);

}

void Graphics::DrawGrid(const debug::GridInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawRing(const debug::RingInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawQuad(const debug::QuadInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::DrawRay(const debug::RayInfo& info)
{
	m_DebugDrawManager->AddTask(info);
}

void Graphics::Culling()
{
	for (auto& object : m_RenderVector)
	{
		std::wstring& fbx = object->FBX;
		std::shared_ptr<ModelData> curFBX = m_ResourceManager->Get<ModelData>(fbx).lock();

		if(!object->isVisible)
			continue;

		if (curFBX != nullptr)
		{
			object->ModelID = curFBX->UID;

			{
				DirectX::SimpleMath::Vector3 s;
				DirectX::SimpleMath::Quaternion r;
				DirectX::SimpleMath::Vector3 t;
				object->world.Decompose(s, r, t);

				DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromQuaternion(r);
				DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(s);

				bool visible = false;
				if (!object->isSkinned)
				{
					for (auto& mesh : curFBX->m_Meshes)
					{
						//S
						DirectX::SimpleMath::Vector3 afterMax = mesh->MaxBounding * s + t;
						DirectX::SimpleMath::Vector3 afterMin = mesh->MinBounding * s + t;

						DirectX::SimpleMath::Vector3 distance = afterMax - afterMin;
						DirectX::SimpleMath::Vector3 half = distance / 2;

						DirectX::BoundingOrientedBox obbInfo;

						obbInfo.Center = t;
						obbInfo.Extents = DirectX::XMFLOAT3(fabs(half.x), fabs(half.y), fabs(half.z));
						obbInfo.Orientation = r;

						DirectX::ContainmentType contains = m_Frustum.Contains(obbInfo);
						if (contains)
						{
							visible |= static_cast<bool>(contains);
							//break
						}
					}
				}
				else
				{
					DirectX::SimpleMath::Vector3 max{ -1,-1,-1 };

					for (auto& mesh : curFBX->m_Meshes)
					{
						DirectX::SimpleMath::Vector3 afterMax = mesh->MaxBounding * s;

						DirectX::SimpleMath::Vector3 afterMin = mesh->MinBounding * s;

						if (afterMax.x > max.x)
						{
							max.x = afterMax.x;
						}

						if (afterMax.y > max.y)
						{
							max.y = afterMax.y;
						}

						if (afterMax.z > max.z)
						{
							max.z = afterMax.z;
						}
					}
					DirectX::BoundingOrientedBox obbInfo;

					obbInfo.Center = t;
					obbInfo.Extents = DirectX::XMFLOAT3(fabs(max.x), fabs(max.y), fabs(max.z));
					obbInfo.Orientation = r;

					DirectX::ContainmentType contains = m_Frustum.Contains(obbInfo);
					if (contains)
					{
						visible |= static_cast<bool>(contains);
						//m_AfterCulling.push_back(object);
						//break;	//바운딩박스보려고 임시 해제
					}
				}

				if (visible && object->isVisible)
				{
					m_AfterCulling.push_back(object);
				}
				//break;
			}
		}
	}
}

std::vector<std::shared_ptr<RenderData>>::iterator Graphics::FindEntity(uint32_t id)
{
	for (auto start = m_RenderVector.begin(); start != m_RenderVector.end(); start++)
	{
		auto entity = *start;
		if (entity->EntityID == id)
		{
			return start;
		}
	}

	return m_RenderVector.end();
}


void Graphics::CreateTextObject(uint32_t entityID, const ui::TextInfo& info)
{
	m_UIManager->CreateTextObject(entityID, info);
}

void Graphics::UpdateTextObject(uint32_t entityID, const ui::TextInfo& info)
{
	m_UIManager->UpdateTextObject(entityID, info);
}

void Graphics::DeleteTextObject(uint32_t entityId)
{
	m_UIManager->DeleteTextObject(entityId);
}

