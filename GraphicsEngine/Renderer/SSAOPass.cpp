#include "SSAOPass.h"

#include "StaticData.h"

#include "ResourceManager.h"
#include "ShaderResourceView.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

#include <random>


SSAOPass::SSAOPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager>resourceManager) : m_KernelSize(0)
{
	m_Device = device;
	m_ResourceManager = resourceManager;

	UINT width = m_Device.lock()->GetWndSize().right - m_Device.lock()->GetWndSize().left;
	UINT height = m_Device.lock()->GetWndSize().bottom - m_Device.lock()->GetWndSize().top;
}

void SSAOPass::Render()
{
	auto resourceManager = m_ResourceManager.lock();
	auto Device = m_Device.lock();

	//일단 SSAO
	std::shared_ptr<RenderTargetView> RTV = resourceManager->Get<RenderTargetView>(L"SSAO").lock();

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

	//set Output Merge
	Device->Context()->OMSetDepthStencilState(nullptr, 1);
	Device->Context()->OMSetRenderTargets(1, RTV->GetAddress(), nullptr);

	//bind IA
	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device.lock()->Context()->IASetVertexBuffers(0, 1, QuadVB->GetAddress(), QuadVB->Size(), QuadVB->Offset());
	m_Device.lock()->Context()->IASetIndexBuffer(QuadIB->Get(), DXGI_FORMAT_R32_UINT, 0);
	Device->Context()->RSSetState(resourceManager->Get<RenderState>(L"Solid").lock()->Get());

	//set VS
	Device->BindVS(QuadVS);
	//set PS
	Device->Context()->PSSetShader(SSAOPS->GetPS(), nullptr, 0);
	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());
	Device->Context()->PSSetConstantBuffers(0, 1, resourceManager->Get<ConstantBuffer<CameraData>>(L"Camera").lock()->GetAddress());
	Device->Context()->PSSetConstantBuffers(1, 1, resourceManager->Get<ConstantBuffer<SSAOKernel>>(L"SSAOKernel").lock()->GetAddress());

	Device->Context()->PSSetConstantBuffers(2, 1, resourceManager->Get<ConstantBuffer<SSAONoise>>(L"SSAONoise").lock()->GetAddress());

	//set SRV
	Device->Context()->PSSetShaderResources(0, 1, depthBuffer->GetAddress());
	Device->Context()->PSSetShaderResources(1, 1, NormalBuffer->GetAddress());
	Device->Context()->PSSetShaderResources(2, 1, PositionBuffer->GetAddress());
	Device->Context()->PSSetShaderResources(3, 1, m_NoiseSRV.lock()->GetAddress());


	Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);

	//unbind resource
	//렌더타겟 해제해줘야지
	Device->UnBindSRV();
	Device->Context()->OMSetRenderTargets(0, nullptr, nullptr);
	Device->Context()->OMSetDepthStencilState(nullptr, 1);
}

void SSAOPass::OnResize()
{

}

void SSAOPass::Initialize()
{
	//반구영역의 차폐판단을 위한 점
	m_KernelSize = 32;

	m_Kernel = std::vector<DirectX::XMFLOAT4>(m_KernelSize);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> XY(-1.0f, 1.0f);
	std::uniform_real_distribution<float> Z(0.0f, 1.0f);

	DirectX::XMFLOAT4* Kernel = m_ResourceManager.lock()->Get<ConstantBuffer<SSAOKernel>>(L"SSAOKernel").lock()->m_struct.sampling;
	for (int i = 0; i < m_KernelSize; i++)
	{
		//난수 생성
		float x = XY(gen);
		float y = XY(gen);
		float z = Z(gen);

		// 정규화
		float length = std::sqrt(x * x + y * y + z * z);
		x /= length;
		y /= length;
		z /= length;

		//반구 중심으로 더 많은 분포를 위한 scale
		float scale = static_cast<float>(i) / m_KernelSize;
		scale = 0.1f + 0.9f * (scale * scale);

		Kernel[i].x = scale * x;
		Kernel[i].y = scale * y;
		Kernel[i].z = scale * z;
		Kernel[i].w = 0;
	}
	m_ResourceManager.lock()->Get<ConstantBuffer<SSAOKernel>>(L"SSAOKernel").lock()->Update();

	//반구를 회전시키기위한 난수 - 밴딩을 최소화하기위해 사용
	m_NoiseSize = 16;
	m_Noise = std::vector<DirectX::XMFLOAT4>(m_NoiseSize * m_NoiseSize);

	for (int i = 0; i < m_NoiseSize * m_NoiseSize; i++)
	{
		m_Noise[i].x = XY(gen);
		m_Noise[i].y = XY(gen);
		m_Noise[i].z = 0.f;
		m_Noise[i].w = 0.f;
	}


	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_NoiseSize;
	texDesc.Height = m_NoiseSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float4
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	m_ResourceManager.lock()->Create<Texture2D>(L"SSAONoise");
	std::shared_ptr<Texture2D> noiseTex = m_ResourceManager.lock()->Get<Texture2D>(L"SSAONoise").lock();

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = m_Noise.data();
	initData.SysMemPitch = m_NoiseSize * sizeof(DirectX::XMFLOAT4);

	m_Device.lock()->Get()->CreateTexture2D(&texDesc, &initData, noiseTex->GetAddress());

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	m_ResourceManager.lock()->Create<ShaderResourceView>(L"SSAONoise");
	std::shared_ptr<ShaderResourceView> noiseSRV = m_ResourceManager.lock()->Get<ShaderResourceView>(L"SSAONoise").lock();
	m_Device.lock()->Get()->CreateShaderResourceView(noiseTex->Get(), &srvDesc, noiseSRV->GetAddress());

	m_NoiseSRV = noiseSRV;
}
