#pragma once
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <queue>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/Effects.h>
#include <directxtk/VertexTypes.h>
#include <wrl/client.h>

#include "vpGraphics.h"

class RenderState;
class DepthStencilState;
class BlendState;
class ResourceManager;
using namespace DirectX;

class Camera;
class Device;

class DebugDrawManager
{
public:
	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager);

	void Execute(const std::shared_ptr<Device>& device, const VPMath::Matrix view,const VPMath::Matrix proj, bool isRender = true);

	void AddTask(const debug::SphereInfo& info) { m_SphereInfos.push(info); }
	void AddTask(const debug::AABBInfo& info) { m_BoxInfos.push(info); }
	void AddTask(const debug::OBBInfo& info) { m_OBBInfos.push(info); }
	void AddTask(const debug::FrustumInfo& info) { m_FrustumInfos.push(info); }
	void AddTask(const debug::GridInfo& info) { m_GridInfos.push(info); }
	void AddTask(const debug::RayInfo& info) { m_RayInfos.push(info); }
	void AddTask(const debug::TriangleInfo& info) { m_TriangleInfos.push(info); }
	void AddTask(const debug::QuadInfo& info) { m_QuadInfos.push(info); }
	void AddTask(const debug::RingInfo& info) { m_RingInfos.push(info); }

private:
	void Draw(const debug::SphereInfo& info);
	void Draw(const debug::AABBInfo& info);
	void Draw(const debug::OBBInfo& info);
	void Draw(const debug::FrustumInfo& info);
	void Draw(const debug::GridInfo& info);
	void Draw(const debug::RayInfo& info);
	void Draw(const debug::TriangleInfo& info);
	void Draw(const debug::QuadInfo& info);

	void DrawRing(const debug::RingInfo& info);
	void DrawCube(const VPMath::Matrix& worldTransform, const VPMath::Color& color);
	void DrawLine(const VPMath::Vector3& start, const VPMath::Vector3& end, const VPMath::Vector4& color);

private:
	std::shared_ptr<BlendState> m_AlphaBlendBS;
	std::shared_ptr<DepthStencilState> m_DefaultDSS;
	std::shared_ptr<RenderState> m_CullNoneRS;

	std::unique_ptr<BasicEffect> m_BatchEffect;
	std::unique_ptr<PrimitiveBatch<VertexPositionColor>> m_Batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_BatchInputLayout;

	// Task Container
	std::queue<debug::SphereInfo> m_SphereInfos;
	std::queue<debug::AABBInfo> m_BoxInfos;
	std::queue<debug::OBBInfo> m_OBBInfos;
	std::queue<debug::FrustumInfo> m_FrustumInfos;
	std::queue<debug::GridInfo> m_GridInfos;
	std::queue<debug::RayInfo> m_RayInfos;
	std::queue<debug::TriangleInfo> m_TriangleInfos;
	std::queue<debug::QuadInfo> m_QuadInfos;
	std::queue<debug::RingInfo> m_RingInfos;
	// etc...
	
};

