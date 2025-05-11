#include "SSAOPass.h"

#include "StaticData.h"

#include "ResourceManager.h"
#include "ShaderResourceView.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

SSAOPass::SSAOPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager>resourceManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;
}

void SSAOPass::Render()
{
	auto resourceManager = m_ResourceManager.lock();
	auto Device= m_Device.lock();

	//일단 메인 출력
	std::shared_ptr<RenderTargetView> RTV = resourceManager->Get<RenderTargetView>(L"RTV_Main").lock();

	//sampling data texture
	auto depthBuffer = resourceManager->Get<ShaderResourceView>(L"Depth").lock();

	auto NormalBuffer = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	auto PositionBuffer = resourceManager->Get<ShaderResourceView>(L"Position").lock();

	auto linear = resourceManager->Get<Sampler>(L"LinearWrap").lock();

	auto QuadVB = resourceManager->Get<VertexBuffer>(L"Quad_VB").lock();
	auto QuadIB = resourceManager->Get<IndexBuffer>(L"Quad_IB").lock();
	auto QuadVS = resourceManager->Get<VertexShader>(L"Quad").lock();
	auto SSAOPS = resourceManager->Get<PixelShader>(L"SSAO").lock();

	Device->UnBindSRV();

	//bind
	Device->BindVS(QuadVS);
	Device->Context()->PSSetShader(SSAOPS->GetPS(),nullptr,0);

	Device->Context()->OMSetDepthStencilState(nullptr, 1);
	Device->Context()->OMSetRenderTargets(1, RTV->GetAddress(),nullptr);

	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device.lock()->Context()->IASetVertexBuffers(0, 1, QuadVB->GetAddress(), QuadVB->Size(), QuadVB->Offset());
	m_Device.lock()->Context()->IASetIndexBuffer(QuadIB->Get(), DXGI_FORMAT_R32_UINT, 0);

	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());
	Device->Context()->RSSetState(resourceManager->Get<RenderState>(L"Solid").lock()->Get());


	Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);

	//렌더타겟 해제해줘야지
	Device->Context()->OMSetRenderTargets(0, nullptr, nullptr);
	Device->Context()->OMSetDepthStencilState(nullptr, 1);
}

void SSAOPass::OnResize()
{

}
