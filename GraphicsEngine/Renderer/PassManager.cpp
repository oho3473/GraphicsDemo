#include "pch.h"
#include "PassManager.h"

#include <memory>

#include "ModelData.h"

#pragma region Manager
#include "ResourceManager.h"
#pragma endregion 

#include <memory>
#include <memory>

#pragma region Pass
#include "TransparencyPass.h"
#include "VPOutLinePass.h"
#include "RimLight.h"
#include "DeferredInstancing.h"
#include "OverDrawPass.h"
#include "DecalPass.h"
#include "DeferredLightPass.h"
#include "EffectPass.h"
#include "OutputMain.h"
#pragma endregion Pass

#include "StaticData.h"
#include "Slot.h"

PassManager::PassManager()
	: m_DeferredPass(std::make_shared<DeferredPass>())
	, m_TransparencyPass(std::make_shared<TransparencyPass>())
	/*, m_DebugPass(std::make_shared<DebugPass>())*/
	/*, m_ObjectMaskPass(std::make_shared<ObjectMaskPass>())*/
	/*, m_ParticlePass(std::make_shared<ParticlePass>())*/
	/*, m_UIPass(std::make_shared<UIPass>())*/
	/*, m_OutlineEdgeDetectPass(std::make_shared<OutlineEdgeDetectPass>())*/
	/*, m_OutlineBlurPass(std::make_shared<OutlineBlurPass>())
	, m_OutlineAddPass(std::make_shared<OutlineAddPass>())*/
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

void PassManager::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resource, const std::shared_ptr<LightManager>& lightmanager, const std::shared_ptr<DecalManager> decalmanager)
{
	m_Device = device;
	m_ResourceManager = resource;
	/*m_DebugDrawManager = debug;
	m_ParticleManager = particleManager;
	m_UIManager = uiManager;*/
	m_LightManager = lightmanager;
	m_DecalManager = decalmanager;

	//offscreen
	/*m_DebugPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_DebugDrawManager.lock());*/
	m_DeferredPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);
	m_TransparencyPass->Initialize(m_Device.lock(), m_ResourceManager.lock());
	/*m_DebugPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_DebugDrawManager.lock());*/
	//m_ObjectMaskPass->Initialize(m_Device.lock(), m_ResourceManager.lock());
	//m_GeometryPass = std::make_shared<GeoMetryPass>(m_Device.lock(), m_ResourceManager.lock());
	m_Instancing = std::make_shared<DeferredInstancing>();
	m_Instancing->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);
	m_OverDraw = std::make_shared<OverDrawPass>(m_Device.lock(), m_ResourceManager.lock());


	m_DeferredLight->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);

	//VPpasses
	m_VPOutLinePass = std::make_shared<VPOutLinePass>(m_Device.lock(), m_ResourceManager.lock());
	m_RimLight = std::make_shared<RimLight>(m_Device.lock(), m_ResourceManager.lock());

	//offscreenpasses
	//m_OutlineEdgeDetectPass->Initialize(m_Device.lock(), m_ResourceManager.lock//());
	//m_OutlineBlurPass->Initialize(m_Device.lock(), m_ResourceManager.lock());
	//m_OutlineAddPass->Initialize(m_Device.lock(), m_ResourceManager.lock());

	//not in passes
	/*m_ParticlePass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_ParticleManager, m_TimeManager);
	m_UIPass->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_UIManager);*/


	//decal
	m_Decal = std::make_shared<DecalPass>(m_Device.lock(), m_ResourceManager.lock(),m_DecalManager);

	//pass push
	//m_OffScreenPasses.push_back(m_DebugPass);
	m_OffScreenPasses.push_back(m_Instancing);	//static
	m_OffScreenPasses.push_back(m_DeferredPass);	//skinned
	//m_OffScreenPasses.push_back(m_ObjectMaskPass);

	m_AfterLightPasses.push_back(m_DeferredLight);
	m_AfterLightPasses.push_back(m_Decal);
	m_AfterLightPasses.push_back(m_TransparencyPass);
	m_AfterLightPasses.push_back(m_OverDraw);

	m_VPPasses.push_back(m_VPOutLinePass);
	m_VPPasses.push_back(m_RimLight);

	//m_IndepentCulling.push_back(m_OutlineEdgeDetectPass);
	//m_IndepentCulling.push_back(m_OutlineBlurPass);
	//m_IndepentCulling.push_back(m_OutlineAddPass);


	m_punch = std::make_shared<EffectPass>(m_Device.lock(), m_ResourceManager.lock());

	m_main = std::make_shared<OutputMain>(m_Device.lock(), m_ResourceManager.lock());

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

	if (m_isVP)
	{
		m_VPOutLinePass->SetRenderQueue(afterCulling);
		m_RimLight->SetRenderQueue(afterCulling);
		m_punch->SetRenderQueue(afterCulling);
	}

	if (!m_isDebugDraw)
	{
		//m_DebugPass->ClearQueue();
	}

}

void PassManager::Render(float deltaTime)
{
	if (m_isDebugDraw)
	{
		//m_DebugPass->Render();
	}

	for (auto& pass : m_OffScreenPasses)
	{
		pass->Render();
	}


	for (auto& pass : m_AfterLightPasses)
	{
		pass->Render();
	}


	if (m_isVP)
	{

		for (auto& pass : m_VPPasses)
		{
			pass->Render();
		}
		m_punch->Render(deltaTime);
	}

	for (auto& pass : m_IndepentCulling)
	{
		pass->Render();
	}


	m_main->Render();
	//m_ParticlePass->Render(deltaTime);
	//m_UIPass->Render();

	// 여태까지는 offscreenRTV에 그리다가 이제 여기서 backbufferRTV에 그린다.
	// DrawIMGUI 이렇게 함수로 두지 말고 FinalPass 클래스로 이식하자.
	//DrawIMGUI();	// 함수 이름이 애매모호하다.
}

void PassManager::OnResize()
{
	m_punch->OnResize();

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

	//m_DebugPass->OnResize();
	m_main->OnResize();

}

void PassManager::SetVP(bool isVP)
{
	m_isVP = isVP;
}

void PassManager::SetDebugDraw(bool on_off)
{
	m_isDebugDraw = on_off;
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
