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


class __declspec(dllexport) IGraphics
{
public:
	virtual ~IGraphics();

	virtual bool Initialize() abstract;
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

	/*
	// ī�޶��� ���� ������Ʈ
	virtual void SetCamera(VPMath::Matrix view, VPMath::Matrix proj, const VPMath::Matrix& orthoProj) abstract;
	//�ø��� ī�޶� ���ü� ������ ����
	virtual void testCulling(VPMath::Matrix view, VPMath::Matrix proj) abstract;
	// �ִϸ��̼��� ����ð�
	virtual const double GetDuration(std::wstring name, int index) abstract;

	// ���� - ���� ��ġ�� �޴´�
	//virtual const VPMath::Matrix Attachment(const uint32_t entityID) abstract;
	virtual const VPMath::Matrix Attachment(const uint32_t entityID, const std::wstring socketName) abstract;

	// vp ��ų ���� ���� �ٲٱ�
	virtual void SetVP(bool isVP) abstract;

	/// Effect
	virtual void CreateParticleObject(uint32_t entityID, const effect::ParticleInfo& info) abstract;
	virtual void UpdateParticleObject(uint32_t entityID, const effect::ParticleInfo& info) abstract;
	virtual void DeleteParticleObjectByID(uint32_t id) abstract;

	/// Light
	virtual void AddLight(uint32_t EntityID, LightType kind, LightData data) abstract;
	virtual void EraseLight(uint32_t EntityID, LightType kind) abstract;
	virtual void UpdateLightData(uint32_t EntityID, LightType kind, LightData data) abstract;

	/// Decal
	virtual void DrawDecal(decal::Info info) abstract;


	/// Debug Draw
	virtual void DrawSphere(const debug::SphereInfo& info) abstract;
	virtual void DrawBox(const debug::AABBInfo& info) abstract;
	virtual void DrawOBB(const debug::OBBInfo& info) abstract;
	virtual void DrawFrustum(const debug::FrustumInfo& info) abstract;
	virtual void DrawGrid(const debug::GridInfo& info) abstract;
	virtual void DrawRing(const debug::RingInfo& info) abstract;
	virtual void DrawTriangle(const debug::TriangleInfo& info) abstract;
	virtual void DrawQuad(const debug::QuadInfo& info) abstract;
	virtual void DrawRay(const debug::RayInfo& info) abstract;

	/// UI
	virtual void CreateImageObject(uint32_t id, const ui::ImageInfo& info) abstract;
	virtual void UpdateImageObject(uint32_t id, const ui::ImageInfo& info) abstract;
	virtual void DeleteImageObject(uint32_t id) abstract;
	virtual void CreateTextObject(uint32_t entityID, const ui::TextInfo& info) abstract;
	virtual void UpdateTextObject(uint32_t entityID, const ui::TextInfo& info) abstract;
	virtual void DeleteTextObject(uint32_t entityId) abstract;
	virtual RECT GetImageRect(uint32_t entityID) const abstract;
	*/


};

extern "C" {
	__declspec(dllexport) IGraphics* CreateInstance(HWND hWnd);
	__declspec(dllexport) void DestroyInstance(IGraphics* instance);
}