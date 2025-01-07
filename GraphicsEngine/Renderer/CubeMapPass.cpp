#include "CubeMapPass.h"
#include "Device.h"

#include "ResourceManager.h"
#include "ViewPort.h"
#include "RenderTargetView.h"
#include "ShaderResourceView.h"
#include "DepthStencilView.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderState.h"
#include "DepthStencilState.h"
#include "RenderData.h"

CubeMapPass::CubeMapPass(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourcemanager) : RenderPass(device,resourcemanager)
{
	m_CubeViewPort = resourcemanager->Get<ViewPort>(L"CubeMapViewPort");
	m_CubeSRV = resourcemanager->Get<ShaderResourceView>(L"CubeMapSRV");
	
	m_CubeDSV = resourcemanager->Get<DepthStencilView>(L"CubeMapDSV");

	m_CubeVB = resourcemanager->Get<VertexBuffer>(L"CubeMap_VB");
	m_CubeIB = resourcemanager->Get<IndexBuffer>(L"CubeMap_IB");

	m_CubeVS = resourcemanager->Get<VertexShader>(L"CubeMapVS");
	m_CubePS = resourcemanager->Get<PixelShader > (L"CubeMapPS");

}

CubeMapPass::~CubeMapPass()
{
	m_CubeRTVs.clear();
}

void CubeMapPass::Render()
{
	std::shared_ptr<Device> device = m_Device.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	device->Context()->IASetInputLayout(m_CubeVS.lock()->InputLayout());
	const DirectX::SimpleMath::Color gray = { 0.15f, 0.15f, 0.15f, 1.f };

	for (int i = 0; i < 6; i++)
	{
		device->UnBindSRV();

		device->BeginRender(m_CubeRTVs[i].lock()->Get(), m_CubeDSV.lock()->Get(), gray);


		device->Context()->RSSetViewports(1, m_CubeViewPort.lock()->Get());
		device->Context()->OMSetRenderTargets(1, m_CubeRTVs[i].lock()->GetAddress(), m_CubeDSV.lock()->Get());
		device->Context()->OMSetDepthStencilState(m_ResourceManager.lock()->Get<DepthStencilState>(L"DisableDepth").lock()->GetState().Get(), 0); // 깊이 비활성화

		device->Context()->IASetIndexBuffer(m_CubeIB.lock()->Get(), DXGI_FORMAT_R32_UINT, 0);
		device->Context()->IASetVertexBuffers(0, 1, m_CubeVB.lock()->GetAddress(), m_CubeVB.lock()->Size(), m_CubeVB.lock()->Offset());
		device->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		device->Context()->VSSetShader(m_CubeVS.lock()->GetVS(), nullptr, 0);
		
		auto maincamera = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera");
		device->Context()->VSSetConstantBuffers(0, 1, maincamera.lock()->GetAddress());

		auto cb = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"CubeCamera");

		cb.lock()->m_struct.view= m_CubeMapCameras[i].view.Transpose();
		cb.lock()->m_struct.viewInverse= m_CubeMapCameras[i].viewInverse;
		cb.lock()->m_struct.proj= m_CubeMapCameras[i].proj.Transpose();

		cb.lock()->Update();
		device->Context()->RSSetState(m_ResourceManager.lock()->Get<RenderState>(L"BackFaceSolid").lock()->Get());
		device->Context()->VSSetConstantBuffers(1, 1, cb.lock()->GetAddress());

	

		device->Context()->PSSetShader(m_CubePS.lock()->GetPS(), nullptr, 0);
		device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

		auto image = m_ResourceManager.lock()->Get<ShaderResourceView>(L"None_metallicRoughness.dds");
		device->Context()->PSSetShaderResources(0, 1, image.lock()->GetAddress());

		device->Context()->DrawIndexed(m_CubeIB.lock()->Count(),0,0);
	
	}

	

	device->Context()->RSSetViewports(1, m_ResourceManager.lock()->Get<ViewPort>(L"Main").lock()->Get());
		device->Context()->RSSetState(m_ResourceManager.lock()->Get<RenderState>(L"Solid").lock()->Get());
		device->Context()->OMSetDepthStencilState(m_ResourceManager.lock()->Get<DepthStencilState>(L"DefaultDSS").lock()->GetState().Get(), 0); // 깊이 비활성화



}

void CubeMapPass::OnResize()
{
	m_CubeDSV = m_ResourceManager.lock()->Get<DepthStencilView>(L"CubeMapDSV");

	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV1"));						   
	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV2"));						   
	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV3"));						   
	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV4"));						   
	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV5"));						   
	m_CubeRTVs.push_back(m_ResourceManager.lock()->Get<RenderTargetView>(L"CubeMapRTV6"));
}

void CubeMapPass::SetCamera(const CameraData* cameras)
{
	for (int i = 0; i < 6; i++)
	{
		m_CubeMapCameras[i] = cameras[i];
	}
}
