#pragma once
#include "ResourceManager.h"
#include "RenderPass.h"


class DeferredLightPass : public RenderPass
{
public:
	~DeferredLightPass() override;
	void Initialize(const std::shared_ptr<Device>& device,
		const std::shared_ptr<ResourceManager>& resourceManager,
		const std::shared_ptr<LightManager>& lightManager);

	void Render()override;
	void OnResize() override;
	void ChangeCubeTex(const std::wstring name);

private:
	std::weak_ptr<LightManager> m_LightManager;
private:
	std::weak_ptr<DepthStencilView> m_DepthStencilView;

	// Multi Render Target
	std::weak_ptr<RenderTargetView> m_AlbedoRTV;
	std::weak_ptr<RenderTargetView> m_NormalRTV;
	std::weak_ptr<RenderTargetView> m_PositionRTV;
	std::weak_ptr<RenderTargetView> m_DepthRTV;
	std::weak_ptr<RenderTargetView> m_MetalicRTV;
	std::weak_ptr<RenderTargetView> m_RoughnessRTV;
	std::weak_ptr<RenderTargetView> m_EmissiveRTV;
	std::weak_ptr<RenderTargetView> m_LightMapRTV;

	//pbr
	std::weak_ptr<RenderTargetView> m_FresnelRTV;
	std::weak_ptr<RenderTargetView> m_DistributeRTV;
	std::weak_ptr<RenderTargetView> m_GeometryRTV;
	std::weak_ptr<RenderTargetView> m_SpecularRTV;
	std::weak_ptr<RenderTargetView> m_DiffuseRTV;
	std::weak_ptr<RenderTargetView> m_GbufferRTV;

	//GBuffer Texture
	std::weak_ptr<ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr<ShaderResourceView> m_NormalSRV;
	std::weak_ptr<ShaderResourceView> m_PositionSRV;
	std::weak_ptr<ShaderResourceView> m_DepthSRV;
	std::weak_ptr<ShaderResourceView> m_MetalicSRV;
	std::weak_ptr<ShaderResourceView> m_RoughnessSRV;
	std::weak_ptr<ShaderResourceView> m_EmissiveSRV;
	std::weak_ptr<ShaderResourceView> m_LightMapSRV;
	std::weak_ptr<ShaderResourceView> m_IrrandianceSRV;
	std::weak_ptr<ShaderResourceView> m_RandianceSRV;
	std::weak_ptr<ShaderResourceView> m_LUT;


	//Light - Quad
	std::weak_ptr<VertexBuffer> m_QuadVB;
	std::weak_ptr<IndexBuffer> m_QuadIB;
	std::weak_ptr<VertexShader> m_QuadVS;
	std::weak_ptr<PixelShader> m_Deferred;

	std::vector<ID3D11RenderTargetView*> m_PBRs;

};

