#pragma once
#include "IGraphics.h" 

#include <unordered_map>
#include <memory>


#pragma region DX
class ViewPort;
class Device;
class ResourceManager;
class ModelLoader;
class Animator;
class RenderTargetView;
class DepthStencilView;
class ShaderResourceView;
#pragma endregion DX

#pragma region Pipeline
class PassManager;
#pragma endregion

#pragma region Manager
class LightManager;
//class DecalManager;
class DebugDrawManager;
class UIManager;
#pragma endregion

class ModelData;

class Graphics : public IGraphics
{
public:
	Graphics(HWND hWnd);
	~Graphics();

	// IGraphics을(를) 통해 상속됨
	virtual bool Initialize() override;
	virtual void CulingUpdate() override;
	virtual void AnimationUpdate(double dt) override;
	virtual void Update(double dt) override;
	virtual void EndUpdate(double dt) override;
	virtual bool Finalize() override;
	virtual void BeginRender() override;
	virtual void Render(float deltaTime) override;
	virtual void EndRender() override;
	virtual void OnResize(HWND hwnd, bool isFullScreen) override;
	virtual void DebugRenderONOFF(bool isRender) override;
	virtual void EraseObject(uint32_t EntityID) override;
	virtual void UpdateModel(uint32_t EntityID) override;
	virtual bool AddRenderModel(std::shared_ptr<RenderData> data) override;
	virtual void SetCamera(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, const DirectX::SimpleMath::Matrix& orthoProj) override;
	virtual void AddLight(uint32_t EntityID, LightType kind, LightData data) override;
	virtual void EraseLight(uint32_t EntityID, LightType kind) override;
	virtual void UpdateLightData(uint32_t EntityID, LightType kind, LightData data) override;
	virtual const double GetDuration(std::wstring name, int index) override;
	virtual void ChangeDebugQuad(bool OnOff, const debug::quadstate state)override;
	virtual void ChangeEnviroment(std::wstring filename)const override;

	virtual void IBLONOFF(bool isRender) override;


	virtual void DrawSphere(const debug::SphereInfo& info)override;
	virtual void DrawAABB(const debug::AABBInfo& info)override;
	virtual void DrawOBB(const debug::OBBInfo& info)override;
	virtual void DrawFrustum(const debug::FrustumInfo& info)override;
	virtual void DrawGrid(const debug::GridInfo& info)override;
	virtual void DrawRing(const debug::RingInfo& info)override;
	virtual void DrawQuad(const debug::QuadInfo& info)override;
	virtual void DrawRay(const debug::RayInfo& info)override;
	virtual void CreateTextObject(uint32_t entityID, const ui::TextInfo& info)override;
	virtual	void UpdateTextObject(uint32_t entityID, const ui::TextInfo& info)override;
	virtual void DeleteTextObject(uint32_t entityId)override;
protected:
	std::vector<std::weak_ptr<RenderTargetView>> m_RTVs;
	std::vector<std::weak_ptr<RenderTargetView>> m_PBRRTVs;
	std::vector<std::weak_ptr<DepthStencilView>> m_DSVs;
	
	std::vector<std::shared_ptr<RenderData>> m_RenderVector;	//프레임워크쪽에서 준 데이터들
	std::vector<std::shared_ptr<RenderData>> m_AfterCulling; //컬링해서 그려낼 최종 친구들
	
	std::unordered_map<uint32_t, LightData> m_Lights;

private:
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<ViewPort> m_CurViewPort;

private:
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::shared_ptr<ModelLoader> m_Loader;
	std::shared_ptr<Animator> m_Animator;
	std::shared_ptr<LightManager> m_LightManager;

	//std::shared_ptr <DecalManager> m_DecalManager;
	std::shared_ptr <DebugDrawManager> m_DebugDrawManager;
	std::shared_ptr <UIManager> m_UIManager;

private:
	HWND m_hWnd;
	RECT m_wndSize;

	//camera
	DirectX::SimpleMath::Matrix m_View;
	DirectX::SimpleMath::Matrix m_Proj;
	DirectX::SimpleMath::Matrix m_ViewProj;
	DirectX::BoundingFrustum m_Frustum;

	CameraData m_CubeMapCameras[6];

	// Pipeline
	std::shared_ptr<PassManager> m_PassManager;

	bool isDebug = false;

private:
	void Culling();
	std::vector<std::shared_ptr<RenderData>>::iterator FindEntity(uint32_t id);

	

};

