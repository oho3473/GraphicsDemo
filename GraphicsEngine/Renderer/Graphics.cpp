#include "pch.h"
#include "Graphics.h"

#pragma region DX
#include <d3d11.h>
#include "Device.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "RenderState.h"
#include "ConstantBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Texture2D.h"

#include "RenderPass.h"
#include "PassManager.h"
#pragma endregion DX

#pragma region Util
#include "Camera.h"
#include "Desc.h"
#include "VertexData.h"
#include "StaticData.h"
#include "ModelData.h"
#include "Mesh.h"
#include "Material.h"
#pragma endregion Util

#pragma region Manager
#include "ResourceManager.h"
#include "ModelLoader.h"
#include "Animator.h"
#include "LightManager.h"
#include "DecalManager.h"
#pragma endregion Manager



Graphics::Graphics(HWND hWnd) : m_hWnd(hWnd)
, m_wndSize()
, m_Device(std::make_shared<Device>())
, m_ResourceManager(std::make_shared<ResourceManager>())
, m_Loader(std::make_shared <ModelLoader>())
, m_LightManager(std::make_shared<LightManager>())
, m_Animator(std::make_shared <Animator>())
, m_DecalManager(std::make_shared <DecalManager>())
, m_PassManager(std::make_shared <PassManager>())
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
	m_PassManager->Initialize(m_Device, m_ResourceManager, m_LightManager, m_DecalManager);

	m_CurViewPort = m_ResourceManager->Create<ViewPort>(L"Main", m_wndSize).lock();

	OnResize(m_hWnd, false);

	return true;
}

void Graphics::CulingUpdate()
{

}

void Graphics::AnimationUpdate(double dt)
{

}

void Graphics::Update(double dt)
{

}

void Graphics::EndUpdate(double dt)
{

}

bool Graphics::Finalize()
{
	return true;
}

void Graphics::BeginRender()
{

}

void Graphics::Render(float deltaTime)
{

}

void Graphics::EndRender()
{

}

void Graphics::OnResize(HWND hwnd, bool isFullScreen)
{

}

void Graphics::DebugRenderONOFF(bool isRender)
{

}

void Graphics::EraseObject(uint32_t EntityID)
{

}

void Graphics::UpdateModel(uint32_t EntityID)
{

}

bool Graphics::AddRenderModel(std::shared_ptr<RenderData> data)
{
	return true;
}

void Graphics::Culling()
{

}

std::vector<std::shared_ptr<RenderData>>::iterator Graphics::FindEntity(uint32_t id)
{

}

