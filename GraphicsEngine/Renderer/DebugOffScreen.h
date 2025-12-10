#pragma once
#include "RenderPass.h"

template<typename T>
class ConstantBuffer;

class DebugOffScreen :
    public RenderPass
{
public:
	DebugOffScreen(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager);

	virtual void Render() override;
	virtual void OnResize() override;
	
	void ChangeQuadState(const debug::quadstate state);

private:
	//Quad
	std::weak_ptr<VertexBuffer> m_QuadVB;
	std::weak_ptr<IndexBuffer> m_QuadIB;
	std::weak_ptr<VertexShader> m_QuadVS;
	std::weak_ptr<PixelShader> m_QuadPS;

	std::weak_ptr<ShaderResourceView> m_Position;
	std::weak_ptr<ShaderResourceView> m_Normal;
	std::weak_ptr<ShaderResourceView> m_Depth;
	std::weak_ptr<ShaderResourceView> m_Metalic;
	std::weak_ptr<ShaderResourceView> m_Roughness;
	std::weak_ptr<ShaderResourceView> m_Albedo;
	std::weak_ptr<ShaderResourceView> m_SSAO;
	std::weak_ptr<ShaderResourceView> m_gbuffer;

	std::weak_ptr<ConstantBuffer<DirectX::XMFLOAT4X4>> m_QuadPos;

	std::vector<std::weak_ptr<ShaderResourceView> > m_SRVs;

	std::weak_ptr<ShaderResourceView> m_Fresnel;
	std::weak_ptr<ShaderResourceView> m_Distribute;
	std::weak_ptr<ShaderResourceView> m_Geometry;
	std::weak_ptr<ShaderResourceView> m_Specular;
	std::weak_ptr<ShaderResourceView> m_Diffuse;
	std::vector<std::weak_ptr<ShaderResourceView> > m_PBRs;

	debug::quadstate m_quadstate;
};

