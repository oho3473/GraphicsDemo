#pragma once
#include "RenderPass.h"
#include "ResourceManager.h"
#include "DecalManager.h"

class DecalPass :
    public RenderPass
{
public:
	DecalPass(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager, const std::shared_ptr<DecalManager> decalmanager);
	~DecalPass();

	virtual void Render() override;
	virtual void OnResize() override;

private:
	std::shared_ptr<DecalManager> m_DecalManager;
private:
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

	std::weak_ptr<VertexBuffer> m_InstanceBuffer;
	std::vector<InstanceDecalData> m_InstanceDatas;

	std::weak_ptr<VertexBuffer> m_DecalVB;
	std::weak_ptr<IndexBuffer> m_DecalIB;

	std::weak_ptr<VertexShader> m_DecalVS;
	std::weak_ptr<PixelShader> m_DecalPS;
};

