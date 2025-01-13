#include "pch.h"
#include "PassManager.h"

#include <memory>

#include "ModelData.h"

#pragma region Manager
#include "ResourceManager.h"
#include "UIManager.h"
#include "LightManager.h"
#pragma endregion 

#include <memory>
#include <memory>

#pragma region Pass
#include "DebugPass.h"
#include "DeferredPass.h"
#include "DeferredInstancing.h"
#include "DeferredLightPass.h"
#include "OutputMain.h"
#include "UIPass.h"
#include "DebugOffScreen.h"
#include "CubeMapPass.h"
#pragma endregion Pass

#include "StaticData.h"
#include "Slot.h"

PassManager::PassManager()
	: m_DeferredPass(std::make_shared<DeferredPass>())
	, m_DebugPass(std::make_shared<DebugPass>())
	, m_UIPass(std::make_shared<UIPass>())
	,m_DeferredLight(std::make_shared<DeferredLightPass>())
{
}

PassManager::~PassManager()
{
	for (auto& pass : m_OffScreenPasses)
	{
		pass.reset();
	}

	m_OffScreenPasses.clear();

	for (auto& pass : m_AfterLightPasses)
	{
		pass.reset();
	}

	m_AfterLightPasses.clear();

	for (auto& pass : m_IndepentCulling)
	{
		pass.reset();
	}

	m_IndepentCulling.clear();

	for (auto& pass : m_VPPasses)
	{
		pass.reset();
	}

	m_VPPasses.clear();

}

void PassManager::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resource, const std::shared_ptr<DebugDrawManager>& debug, const std::shared_ptr<LightManager>& lightmanager,const std::shared_ptr<UIManager>& uiManager)
{
	m_Device = device;
	m_ResourceManager = resource;
	m_DebugDrawManager = debug;
	m_UIManager = uiManager;
	m_LightManager = lightmanager;

	//offscreen
	m_DebugPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_DebugDrawManager.lock());
	m_DeferredPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);
	m_DebugPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_DebugDrawManager.lock());


	m_DeferredLight->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);

	//offscreenpasses
	m_UIPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_UIManager);


	//pass push
	m_OffScreenPasses.push_back(m_DeferredPass);	//geometry
	m_AfterLightPasses.push_back(m_DeferredLight);	//deferred light

	m_MainOutput = std::make_shared<OutputMain>(m_Device.lock(), m_ResourceManager.lock());

	m_DebugOffScreen = std::make_shared<DebugOffScreen>(m_Device.lock(), m_ResourceManager.lock());

	m_CubeMap = std::make_shared<CubeMapPass>(m_Device.lock(), m_ResourceManager.lock());
}

void PassManager::Update(const std::vector<std::shared_ptr<RenderData>>& afterCulling)
{

	for (auto& pass : m_OffScreenPasses)
	{
		pass->SetRenderQueue(afterCulling);
	}

	for (auto& pass : m_AfterLightPasses)
	{
		pass->SetRenderQueue(afterCulling);
	}
		
	if (!m_isDebugDraw)
	{
		m_DebugPass->ClearQueue();
	}
}

void PassManager::Render(float deltaTime)
{
	m_CubeMap->Render();
	if (m_isDebugDraw)
	{
		m_DebugPass->Render();
	}

	for (auto& pass : m_OffScreenPasses)
	{
		pass->Render();
	}


	for (auto& pass : m_AfterLightPasses)
	{
		pass->Render();
	}

	for (auto& pass : m_IndepentCulling)
	{
		pass->Render();
	}
	if (m_isDebugDraw)
	{
		m_DebugOffScreen->Render();
	}
	m_UIPass->Render();
	m_MainOutput->Render();
}

void PassManager::OnResize()
{

	for (auto& pass : m_OffScreenPasses)
	{
		pass->OnResize();
	}

	for (auto& pass : m_AfterLightPasses)
	{
		pass->OnResize();
	}

	for (auto& pass : m_VPPasses)
	{
		pass->OnResize();
	}

	for (auto& pass : m_IndepentCulling)
	{
		pass->OnResize();
	}

	m_CubeMap->OnResize();
	m_DebugPass->OnResize();
	m_DebugOffScreen->OnResize();
	m_MainOutput->OnResize();
}

void PassManager::IBLOnOff(bool isRender)
{
	m_IBL = isRender;
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> useIBL = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"Color").lock();

	useIBL->Update({ static_cast<float>(isRender),0,0,0});


}

void PassManager::SetDebugDraw(bool on_off)
{
	m_isDebugDraw = on_off;
}

void PassManager::SetCubeCamera(const CameraData* datas)
{
	m_CubeMap->SetCamera(datas);
}

void PassManager::DrawIMGUI()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<Sampler> linear = resourcemanager->Get<Sampler>(L"LinearWrap").lock();
	std::shared_ptr<VertexBuffer> vb = resourcemanager->Get<VertexBuffer>(L"Quad_VB").lock();
	std::shared_ptr<IndexBuffer> ib = resourcemanager->Get<IndexBuffer>(L"Quad_IB").lock();
	std::shared_ptr<PixelShader> ps = resourcemanager->Get<PixelShader>(L"Quad").lock();
	std::shared_ptr<VertexShader> vs = resourcemanager->Get<VertexShader>(L"Quad").lock();
	std::shared_ptr<ShaderResourceView> gui = resourcemanager->Get<ShaderResourceView>(L"GBuffer").lock();
	std::shared_ptr<RenderTargetView> rtv = resourcemanager->Get<RenderTargetView>(L"RTV_Main").lock();
	std::shared_ptr<DepthStencilView> dsv = resourcemanager->Get<DepthStencilView>(L"DSV_Main").lock();

	Device->UnBindSRV();

	Device->Context()->IASetInputLayout(vs->InputLayout());
	Device->Context()->VSSetShader(vs->GetVS(), nullptr, 0);
	Device->Context()->PSSetShader(ps->GetPS(), nullptr, 0);

	Device->Context()->RSSetState(resourcemanager->Get<RenderState>(L"Solid").lock()->Get());

	m_Device.lock()->Context()->IASetVertexBuffers(0, 1, vb->GetAddress(), vb->Size(), vb->Offset());
	m_Device.lock()->Context()->IASetIndexBuffer(ib->Get(), DXGI_FORMAT_R32_UINT, 0);

	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::GBuffer), 1, gui->GetAddress());

	Device->Context()->PSSetSamplers(static_cast<UINT>(Slot_S::Linear), 1, linear->GetAddress());

	Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), nullptr);
	Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);
}
