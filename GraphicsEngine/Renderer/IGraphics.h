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

	virtual void AddLight(uint32_t EntityID, LightType kind, LightData data) abstract;
	virtual void EraseLight(uint32_t EntityID, LightType kind) abstract;
	virtual void UpdateLightData(uint32_t EntityID, LightType kind, LightData data) abstract;
};

extern "C" {
	__declspec(dllexport) IGraphics* CreateGraphics(HWND hWnd);
	__declspec(dllexport) void DestroyGraphics(IGraphics* instance);
}