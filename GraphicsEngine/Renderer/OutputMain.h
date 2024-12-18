#pragma once
#include "RenderPass.h"
#include "ConstantBuffer.h"



class OutputMain :
    public RenderPass
{
public:
	OutputMain(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager);
	~OutputMain();

	virtual void Render() override;
	virtual void OnResize() override;
private:
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_Screen;

	//Quad
	std::weak_ptr<VertexBuffer> m_QuadVB;
	std::weak_ptr<IndexBuffer> m_QuadIB;
	std::weak_ptr<VertexShader> m_QuadVS;
	std::weak_ptr<PixelShader> m_ps;


	std::weak_ptr<ShaderResourceView> m_gbuffer;
	std::weak_ptr<ShaderResourceView> m_Depth;

};

