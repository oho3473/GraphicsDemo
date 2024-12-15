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

class LightManager;
class DecalManager;

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
protected:
	std::vector<std::weak_ptr<RenderTargetView>> m_RTVs;
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

	std::shared_ptr <DecalManager> m_DecalManager;

private:
	HWND m_hWnd;
	RECT m_wndSize;

	//camera
	Matrix m_View;
	Matrix m_Proj;
	Matrix m_ViewProj;
	DirectX::BoundingFrustum m_Frustum;

	// Pipeline
	std::shared_ptr<PassManager> m_PassManager;

	bool isDebug = false;

private:
	void Culling();
	std::vector<std::shared_ptr<RenderData>>::iterator FindEntity(uint32_t id);
};

