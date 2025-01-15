#pragma once
#include "RenderPass.h"

class LightManager;

class DeferredPass : public RenderPass
{
public:
	~DeferredPass() override;

	void Initialize(const std::shared_ptr<Device>& device,
		const std::shared_ptr<ResourceManager>& resourceManager,
		const std::shared_ptr<LightManager>& lightManager);
	void Render() override;
	void OnResize() override;

private:

private:
	std::weak_ptr<LightManager> m_LightManager;
private:
	std::weak_ptr<DepthStencilView> m_DepthStencilView;

	//GeometryPass
	std::weak_ptr<PixelShader> m_GeometryPS;

	// Multi Render Target
	std::weak_ptr<RenderTargetView> m_AlbedoRTV;
	std::weak_ptr<RenderTargetView> m_NormalRTV;
	std::weak_ptr<RenderTargetView> m_PositionRTV;
	std::weak_ptr<RenderTargetView> m_DepthRTV;
	std::weak_ptr<RenderTargetView> m_MetalicRTV;
	std::weak_ptr<RenderTargetView> m_RoughnessRTV;
	std::weak_ptr<RenderTargetView> m_EmissiveRTV;
	std::weak_ptr<RenderTargetView> m_LightMapRTV;

	//GBuffer Texture
	std::weak_ptr<ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr<ShaderResourceView> m_NormalSRV;
	std::weak_ptr<ShaderResourceView> m_PositionSRV;
	std::weak_ptr<ShaderResourceView> m_DepthSRV;
	std::weak_ptr<ShaderResourceView> m_MetalicSRV;
	std::weak_ptr<ShaderResourceView> m_RoughnessSRV;
	std::weak_ptr<ShaderResourceView> m_EmissiveSRV;
	std::weak_ptr<ShaderResourceView> m_LightMapSRV;

	//GBuffer Result
	std::weak_ptr<ShaderResourceView> m_GBufferSRV;
};

