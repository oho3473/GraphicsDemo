#pragma once
#include <memory>
#include "ConstantBuffer.h"
#include "Device.h"
#include "VertexShader.h"
#include "ModelData.h"

#include <string>
#include <queue>
#include <wrl/client.h>

#include "Slot.h"

enum class PassState : unsigned;
class Mesh;
class RenderState;
class ResourceManager;

class DeferredGeometryPass
{
public:
	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,
		const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

	void Render(const std::shared_ptr<RenderData>& model);

private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;
	

	std::shared_ptr<DepthStencilView> m_DepthStencilView;

	// Shader
	std::shared_ptr<VertexShader> m_StaticMeshVS;
	std::shared_ptr<VertexShader> m_SkeletalMeshVS;
	
	std::shared_ptr<PixelShader> m_GeometryPS;
	std::shared_ptr<PixelShader> m_StaticPS;
	std::shared_ptr<PixelShader> m_SkinningPS;

	// Multi Render Target
	enum { GBufferSize = static_cast<int>(Slot_B::End)};	// 상수. #define 보다 디버깅할때 더 편하다.
	std::shared_ptr<RenderTargetView> m_AlbedoRTV;
	std::shared_ptr<RenderTargetView> m_NormalRTV;
	std::shared_ptr<RenderTargetView> m_PositionRTV;
	std::shared_ptr<RenderTargetView> m_DepthRTV;
	std::shared_ptr<RenderTargetView> m_MetalicRTV;
	std::shared_ptr<RenderTargetView> m_RoughnessRTV;
	std::shared_ptr<RenderTargetView> m_AORTV;
	std::shared_ptr<RenderTargetView> m_EmissiveRTV;

	// Constant Buffers
	std::shared_ptr<ConstantBuffer<TransformData>> m_TransformCB;
	std::shared_ptr<ConstantBuffer<MatrixPallete>> m_SkeletalCB;
	std::weak_ptr<ConstantBuffer<CameraData>> m_CameraCB;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSS_Null;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BS_Null;

	// Camera Data.. 임시
	DirectX::SimpleMath::Matrix m_View;
	DirectX::SimpleMath::Matrix m_Proj;
};
