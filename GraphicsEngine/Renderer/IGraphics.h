#pragma once
#include <windows.h>

#pragma region STL
#include <string>
#include <memory> //smart pointer
#include <vector>
#include <map>
#include <queue>
#pragma endregion STL

#include "RenderData.h"
#include "BufferData.h"

#ifdef RENDERDATA_EXPORTS
#define RENDERDATA_API __declspec(dllexport)
#else
#define RENDERDATA_API __declspec(dllimport)
#endif


class  IGraphics
{
public:
	__declspec(dllexport) virtual ~IGraphics();

	__declspec(dllexport) virtual bool Initialize() abstract;

	virtual void CulingUpdate() abstract;
	virtual void AnimationUpdate(double dt) abstract;
	virtual void Update(double dt) abstract;
	virtual void EndUpdate(double dt) abstract;

	virtual bool Finalize() abstract;
	virtual void BeginRender() abstract;
	virtual void Render(float deltaTime) abstract;
	virtual void EndRender() abstract;

	virtual void OnResize(HWND hwnd, bool isFullScreen) abstract;

	virtual void DebugRenderONOFF(bool isRender) abstract;

	virtual void EraseObject(uint32_t EntityID) abstract;
	virtual void UpdateModel(uint32_t EntityID) abstract;
	virtual bool AddRenderModel(std::shared_ptr<RenderData> data) abstract;

	virtual void SetCamera(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj, const DirectX::SimpleMath::Matrix& orthoProj) abstract;
	virtual void SetCubeCamera(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix pro, int index) abstract;

	virtual void AddLight(uint32_t EntityID, LightType kind, LightData data) abstract;
	virtual void EraseLight(uint32_t EntityID, LightType kind) abstract;
	virtual void UpdateLightData(uint32_t EntityID, LightType kind, LightData data) abstract;

	virtual const double GetDuration(std::wstring name, int index) abstract;

	virtual void DrawSphere(const debug::SphereInfo& info)abstract;
	virtual void DrawAABB(const debug::AABBInfo& info)abstract;
	virtual void DrawOBB(const debug::OBBInfo& info)abstract;
	virtual void DrawFrustum(const debug::FrustumInfo& info)abstract;
	virtual void DrawGrid(const debug::GridInfo& info)abstract;
	virtual void DrawRing(const debug::RingInfo& info)abstract;
	virtual void DrawQuad(const debug::QuadInfo& info)abstract;
	virtual void DrawRay(const debug::RayInfo& info)abstract;

	virtual void CreateTextObject(uint32_t entityID, const ui::TextInfo& info)abstract;
	virtual	void UpdateTextObject(uint32_t entityID, const ui::TextInfo& info)abstract;
	virtual void DeleteTextObject(uint32_t entityId)abstract;
};

extern "C" {
	__declspec(dllexport) IGraphics* CreateGraphics(HWND hWnd);
	__declspec(dllexport) void DestroyGraphics(IGraphics* instance);
}