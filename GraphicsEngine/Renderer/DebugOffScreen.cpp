#include "DebugOffScreen.h"
#include "ResourceManager.h"
#include "Slot.h"
#include "Sampler.h"
#include "StaticData.h"
#include "ConstantBuffer.h"

DebugOffScreen::DebugOffScreen(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager) : m_quadstate(debug::quadstate::GEOMETRY)
{
	m_ResourceManager = manager;
	m_Device = device;

	m_QuadVB = manager->Get<VertexBuffer>(L"DebugQuad_VB");
	m_QuadIB = manager->Get<IndexBuffer>(L"DebugQuad_IB");
	m_QuadVS = manager->Get<VertexShader>(L"DebugQuad");
	m_QuadPS = manager->Get<PixelShader>(L"DebugQuad");
	m_QuadPos = manager->Get<ConstantBuffer<DirectX::XMFLOAT4X4>>(L"QuadPos");

	m_Position = manager->Get<ShaderResourceView>(L"Position").lock();
	m_Normal = manager->Get<ShaderResourceView>(L"Normal").lock();
	m_Depth = manager->Get<ShaderResourceView>(L"Depth").lock();
	m_Metalic = manager->Get<ShaderResourceView>(L"Metalic").lock();
	m_Roughness = manager->Get<ShaderResourceView>(L"Roughness").lock();
	m_Albedo = manager->Get<ShaderResourceView>(L"Albedo").lock();

	m_SRVs.push_back(m_Position);
	m_SRVs.push_back(m_Normal);
	m_SRVs.push_back(m_Metalic);
	m_SRVs.push_back(m_Roughness);
	m_SRVs.push_back(m_Depth);
	m_SRVs.push_back(m_Albedo);
}

void DebugOffScreen::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<VertexBuffer> vb = m_QuadVB.lock();
	std::shared_ptr<IndexBuffer> ib = m_QuadIB.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	UnBindResource();


	Device->BindVS(m_QuadVS.lock());
	Device->Context()->PSSetShader(m_QuadPS.lock()->GetPS(), nullptr, 0);

	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Device->Context()->RSSetState(resourcemanager->Get<RenderState>(L"Solid").lock()->Get());

	m_Device.lock()->Context()->IASetVertexBuffers(0, 1, vb->GetAddress(), vb->Size(), vb->Offset());
	m_Device.lock()->Context()->IASetIndexBuffer(ib->Get(), DXGI_FORMAT_R32_UINT, 0);

	std::shared_ptr<RenderTargetView> rtv = resourcemanager->Get<RenderTargetView>(L"GBuffer").lock();

	Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), nullptr);
	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

	DirectX::XMFLOAT4X4 pos{};
	float scale = 1 / static_cast<float>(m_SRVs.size());
	pos._11 = scale; pos._22 = scale; pos._33 = scale; pos._44 = 1.f;

	switch (m_quadstate)
	{
		case debug::quadstate::GEOMETRY:
		{
			for (int i = 0; i < m_SRVs.size(); i++)
			{
				m_QuadPos.lock()->m_struct;

				pos._14 = 1 - scale; pos._24 = (1 - scale) - i * scale * 2;
				m_QuadPos.lock()->Update(pos);

				Device->Context()->VSSetConstantBuffers(0, 1, m_QuadPos.lock()->GetAddress());

				Device->Context()->PSSetShaderResources(0, 1, m_SRVs[i].lock()->GetAddress());

				Device->Context()->DrawIndexed(DebugQuad::Index::count, 0, 0);
			}
		}
		break;
		case debug::quadstate::PBR:
		{
			for (int i = 0; i < m_PBRs.size(); i++)
			{
				m_QuadPos.lock()->m_struct;

				pos._14 = 1 - scale; pos._24 = (1 - scale) - i * scale * 2;
				m_QuadPos.lock()->Update(pos);

				Device->Context()->VSSetConstantBuffers(0, 1, m_QuadPos.lock()->GetAddress());

				Device->Context()->PSSetShaderResources(0, 1, m_PBRs[i].lock()->GetAddress());

				Device->Context()->DrawIndexed(DebugQuad::Index::count, 0, 0);
			}
		}
			break;
		default:
			break;
	}

	UnBindResource();

}

void DebugOffScreen::OnResize()
{
	std::shared_ptr<ResourceManager> manager = m_ResourceManager.lock();

	const uint32_t width = m_Device.lock()->GetWndWidth();
	const uint32_t height = m_Device.lock()->GetWndHeight();

	m_gbuffer = manager->Get<ShaderResourceView>(L"GBuffer").lock();
	m_Position = manager->Get<ShaderResourceView>(L"Position").lock();
	m_Normal = manager->Get<ShaderResourceView>(L"Normal").lock();
	m_Depth = manager->Get<ShaderResourceView>(L"Depth").lock();
	m_Metalic = manager->Get<ShaderResourceView>(L"Metalic").lock();
	m_Roughness = manager->Get<ShaderResourceView>(L"Roughness").lock();
	m_Albedo = manager->Get<ShaderResourceView>(L"Albedo").lock();
	m_SSAO = manager->Get<ShaderResourceView>(L"SSAO").lock();

	m_QuadVB = manager->Get<VertexBuffer>(L"Quad_VB");
	m_QuadIB = manager->Get<IndexBuffer>(L"Quad_IB");
	m_QuadVS = manager->Get<VertexShader>(L"DebugQuad");
	m_QuadPS = manager->Get<PixelShader>(L"DebugQuad");
	m_QuadPos = manager->Get<ConstantBuffer<DirectX::XMFLOAT4X4>>(L"QuadPos");
	

	m_SRVs.clear();

	m_SRVs.push_back(m_Normal);
	m_SRVs.push_back(m_Metalic);
	m_SRVs.push_back(m_Roughness);
	m_SRVs.push_back(m_Depth);
	m_SRVs.push_back(m_Albedo);
	//m_SRVs.push_back(m_SSAO);


	m_Fresnel = manager->Get<ShaderResourceView>(L"Fresnel").lock();
	m_Distribute = manager->Get<ShaderResourceView>(L"Distribute").lock();
	m_Geometry = manager->Get<ShaderResourceView>(L"Geometry").lock();
	m_Specular = manager->Get<ShaderResourceView>(L"Specular").lock();
	m_Diffuse = manager->Get<ShaderResourceView>(L"Diffuse").lock();

	m_PBRs.clear();

	m_PBRs.push_back(m_Fresnel);
	m_PBRs.push_back(m_Distribute);
	m_PBRs.push_back(m_Geometry);
	m_PBRs.push_back(m_Specular);
	m_PBRs.push_back(m_Diffuse);
}

void DebugOffScreen::ChangeQuadState(const debug::quadstate state)
{
	m_quadstate = state;
}
