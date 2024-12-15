#pragma once
#include "RenderPass.h"
#include "SimpleMath.h"

class RenderData;

template<typename T>
class ConstantBuffer;

class EffectPass
{
public:
	EffectPass(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager);
	~EffectPass();

	void Render(float deltaTime);
	void OnResize();
	void SetRenderQueue(const std::vector<std::shared_ptr<RenderData>>& renderQueue);
private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;

	std::weak_ptr<DepthStencilView> m_DepthStencilView;

	//GBuffer Texture
	std::weak_ptr<ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr<ShaderResourceView> m_NormalSRV;
	std::weak_ptr<ShaderResourceView> m_PositionSRV;
	std::weak_ptr<ShaderResourceView> m_DepthSRV;
	std::weak_ptr<ShaderResourceView> m_MetalicRoughnessSRV;
	std::weak_ptr<ShaderResourceView> m_AmbientOcclusionSRV;
	std::weak_ptr<ShaderResourceView> m_EmissiveSRV;
	std::weak_ptr<ShaderResourceView> m_LightMapSRV;

	//quad
	std::weak_ptr<VertexBuffer> m_QuadVB;
	std::weak_ptr<IndexBuffer> m_QuadIB;
	std::weak_ptr<VertexShader> m_QuadVS;
	std::weak_ptr<PixelShader> m_QuadPS;

	//GBuffer Result
	std::weak_ptr<ShaderResourceView> m_GBufferSRV;
	std::weak_ptr<ShaderResourceView> m_NormalCopySRV;
	std::weak_ptr<ShaderResourceView> m_NoiseSRV;


	// Multi Render Target
	std::weak_ptr<RenderTargetView> m_AlbedoRTV;
	std::weak_ptr<RenderTargetView> m_NormalRTV;
	std::weak_ptr<RenderTargetView> m_PositionRTV;
	std::weak_ptr<RenderTargetView> m_DepthRTV;
	std::weak_ptr<RenderTargetView> m_MetalicRoughnessRTV;
	std::weak_ptr<RenderTargetView> m_AORTV;
	std::weak_ptr<RenderTargetView> m_EmissiveRTV;
	std::weak_ptr<RenderTargetView> m_LightMapRTV;
	std::weak_ptr<RenderTargetView> m_GBuffer;
	std::weak_ptr<RenderTargetView> m_NormalCopyRTV;

	std::weak_ptr<VertexShader> m_StaticMeshVS;
	std::weak_ptr<PixelShader> m_PuchPS;


	std::weak_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_TimeCB;
	std::vector<std::shared_ptr<RenderData>> m_RenderList;

};

