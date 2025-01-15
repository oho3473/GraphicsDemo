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
#include "DebugPBRPass.h"
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

	m_DebugPBR = std::make_shared<DebugPBRPass>();
	m_DebugPBR->Initialize(m_Device.lock(), m_ResourceManager.lock(), m_LightManager);
	m_OffScreenPasses.push_back(m_DebugPBR);
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

void PassManager::ChageDebugQuad(const debug::quadstate state)
{
	m_DebugOffScreen->ChangeQuadState(state);
}

void PassManager::SetDebugDraw(bool on_off)
{
	m_isDebugDraw = on_off;
}

void PassManager::SetCubeCamera(const CameraData* datas)
{
	m_CubeMap->SetCamera(datas);
}
