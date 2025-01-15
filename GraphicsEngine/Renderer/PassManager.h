#pragma once
#include "RenderData.h"
#include "BufferData.h"



/// <summary>
///	모든 패스들을 관리하는 매니저.
/// 디퍼드 렌더링 기법을 사용한다.
/// </summary>
#include <memory>

class Device;
class ModelData;
class RenderPass;
#pragma region Pass
class DeferredInstancing;
class DeferredLightPass;
class OutputMain;
class UIPass;
class DebugOffScreen;
class DebugPass;
class DeferredPass;
class CubeMapPass;
class DebugPBRPass;
#pragma region Pass

#pragma region Manager
class LightManager;
class UIManager;
class ResourceManager;
class DebugDrawManager;
#pragma endregion
	
class PassManager
{
public:
	PassManager();
	~PassManager();

	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resource, const std::shared_ptr<DebugDrawManager>& debug, const std::shared_ptr<LightManager>& lightmanager, const std::shared_ptr<UIManager>& uiManager);
	void Update(const std::vector<std::shared_ptr<RenderData>>& afterCulling);

	void Render(float deltaTime);
	void OnResize();
	void ChageDebugQuad(const debug::quadstate state);
	void SetDebugDraw(bool on_off);
	void SetCubeCamera(const CameraData* datas);

private:

private:
	std::vector<std::shared_ptr<RenderPass>> m_OffScreenPasses;	//기본 depth, normal... offscreen
	std::vector<std::shared_ptr<RenderPass>> m_AfterLightPasses;	//postProcessing
	std::vector<std::shared_ptr<RenderPass>> m_VPPasses;	//vp 상태일떄만 쓰는 패스
	std::vector<std::shared_ptr<RenderPass>> m_IndepentCulling;


	std::shared_ptr<UIPass> m_UIPass;
	std::shared_ptr<DeferredPass> m_DeferredPass;
	std::shared_ptr<DebugPass> m_DebugPass;
	std::shared_ptr<DeferredInstancing> m_Instancing;
	std::shared_ptr<DeferredLightPass> m_DeferredLight;
	std::shared_ptr<OutputMain> m_MainOutput;
	std::shared_ptr<DebugOffScreen> m_DebugOffScreen;
	std::shared_ptr<DebugPBRPass> m_DebugPBR;
	std::shared_ptr<CubeMapPass> m_CubeMap;

private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;
	std::weak_ptr<DebugDrawManager> m_DebugDrawManager;
	std::shared_ptr<UIManager> m_UIManager;
	std::shared_ptr<LightManager> m_LightManager;

	DirectX::SimpleMath::Matrix m_View;
	DirectX::SimpleMath::Matrix m_Proj;

private:
	bool m_IBL = false; 
	bool m_isDebugDraw = false;
};

