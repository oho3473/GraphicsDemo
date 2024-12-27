#pragma once
#include "RenderData.h"

#include "TransparencyPass.h"
#include "DebugPass.h"
#include "DeferredPass.h"


/// <summary>
///	모든 패스들을 관리하는 매니저.
/// 디퍼드 렌더링 기법을 사용한다.
/// </summary>

class ModelData;

#pragma region Pass
class VPOutLinePass;
class RimLight;
class DeferredInstancing;
class OverDrawPass;
class DecalPass;
class DeferredLightPass;
class EffectPass;
class OutputMain;
#pragma region Pass

#pragma region Manager
class DecalManager;
class LightManager;
#pragma endregion
	
class PassManager
{
public:
	PassManager();
	~PassManager();

	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resource, const std::shared_ptr<DebugDrawManager>& debug, const std::shared_ptr<LightManager>& lightmanager,
		const std::shared_ptr<DecalManager> decalmanager);
	void Update(const std::vector<std::shared_ptr<RenderData>>& afterCulling);

	void Render(float deltaTime);
	void OnResize();
	void SetVP(bool isVP);
	void SetDebugDraw(bool on_off);


private:
	void DrawIMGUI();

private:
	std::vector<std::shared_ptr<RenderPass>> m_OffScreenPasses;	//기본 depth, normal... offscreen
	std::vector<std::shared_ptr<RenderPass>> m_AfterLightPasses;	//postProcessing
	std::vector<std::shared_ptr<RenderPass>> m_VPPasses;	//vp 상태일떄만 쓰는 패스
	std::vector<std::shared_ptr<RenderPass>> m_IndepentCulling;

	std::shared_ptr<DeferredPass> m_DeferredPass;
	std::shared_ptr<TransparencyPass> m_TransparencyPass;
	std::shared_ptr<DebugPass> m_DebugPass;
	std::shared_ptr<VPOutLinePass> m_VPOutLinePass;
	std::shared_ptr<RimLight> m_RimLight;
	std::shared_ptr<DeferredInstancing> m_Instancing;
	std::shared_ptr<OverDrawPass> m_OverDraw;
	std::shared_ptr<DecalPass> m_Decal;
	std::shared_ptr<DeferredLightPass> m_DeferredLight;
	std::shared_ptr<EffectPass> m_punch;
	std::shared_ptr<OutputMain> m_main;

private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;
	std::weak_ptr<DebugDrawManager> m_DebugDrawManager;
	//std::shared_ptr<ParticleManager> m_ParticleManager;
	//TimeManager* m_TimeManager = nullptr;
	//std::shared_ptr<UIManager> m_UIManager;
	std::shared_ptr<LightManager> m_LightManager;
	std::shared_ptr<DecalManager> m_DecalManager;

	DirectX::SimpleMath::Matrix m_View;
	DirectX::SimpleMath::Matrix m_Proj;

private:
	bool m_isVP = false; 
	bool m_isDebugDraw = false;
};

