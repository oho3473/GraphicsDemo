#pragma once
#include "RenderPass.h"
#include "VertexData.h"

class LightManager;


class DeferredInstancing :
    public RenderPass
{
public:
	~DeferredInstancing() override;

	void Initialize(const std::shared_ptr<Device>& device,
		const std::shared_ptr<ResourceManager>& resourceManager,
		const std::shared_ptr<LightManager>& lightManager);
	void Render() override;
	void OnResize() override;
	void SetRenderQueue(const std::vector<std::shared_ptr<RenderData>>& renderQueue) override;

private:
	void DrawStatic();
	void DrawSkinned();


	std::weak_ptr<VertexShader> m_InstancingVS;
	std::weak_ptr<VertexBuffer> m_InstanceBuffer;
	std::vector<InstanceData> m_InstanceDatas;

	std::weak_ptr<VertexShader> m_InstancingSkinnedVS;
	std::weak_ptr<VertexBuffer> m_InstanceSkinnedBuffer;
	std::vector<InstanceSkinnedData> m_InstanceSkinnedDatas;

	std::queue<std::pair<int, int>> m_instancecount;
	std::queue<std::pair<int, int>> m_instanceskinnedcount;

private:
	std::weak_ptr<LightManager> m_LightManager;

	std::weak_ptr<DepthStencilView> m_DepthStencilView;

	std::weak_ptr<PixelShader> m_SkinnePS;

	// Multi Render Target
	std::weak_ptr<RenderTargetView> m_AlbedoRTV;
	std::weak_ptr<RenderTargetView> m_NormalRTV;
	std::weak_ptr<RenderTargetView> m_PositionRTV;
	std::weak_ptr<RenderTargetView> m_DepthRTV;
	std::weak_ptr<RenderTargetView> m_MetalicRoughnessRTV;
	std::weak_ptr<RenderTargetView> m_AORTV;
	std::weak_ptr<RenderTargetView> m_EmissiveRTV;
	std::weak_ptr<RenderTargetView> m_LightMapRTV;

	//GBuffer Texture
	std::weak_ptr<ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr<ShaderResourceView> m_NormalSRV;
	std::weak_ptr<ShaderResourceView> m_PositionSRV;
	std::weak_ptr<ShaderResourceView> m_DepthSRV;
	std::weak_ptr<ShaderResourceView> m_MetalicRoughnessSRV;
	std::weak_ptr<ShaderResourceView> m_AmbientOcclusionSRV;
	std::weak_ptr<ShaderResourceView> m_EmissiveSRV;
	std::weak_ptr<ShaderResourceView> m_LightMapSRV;

	//Light - Quad
	std::weak_ptr<VertexBuffer> m_QuadVB;
	std::weak_ptr<IndexBuffer> m_QuadIB;
	std::weak_ptr<VertexShader> m_QuadVS;
	std::weak_ptr<PixelShader> m_QuadPS;
	std::weak_ptr<PixelShader> m_Deferred;

	//GBuffer Result
	std::weak_ptr<ShaderResourceView> m_GBufferSRV;

};

