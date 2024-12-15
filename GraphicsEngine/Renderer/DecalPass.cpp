#include "pch.h"
#include "DecalPass.h"
#include "Slot.h"
#include "StaticData.h"
#include "BlendState.h"
#include "DepthStencilState.h"

DecalPass::DecalPass(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager, const std::shared_ptr<DecalManager> decalmanager) : RenderPass(device, resourceManager)
{
	m_DecalManager = decalmanager;

	m_Device = device;
	m_ResourceManager = resourceManager;
	decalmanager->Initialize(m_ResourceManager);

	m_DepthStencilView = resourceManager->Get<DepthStencilView>(L"DSV_Deferred").lock();

	m_AlbedoRTV = resourceManager->Get<RenderTargetView>(L"Albedo").lock();
	m_NormalRTV = resourceManager->Get<RenderTargetView>(L"Normal").lock();
	m_DepthRTV = resourceManager->Get<RenderTargetView>(L"Depth").lock();
	m_MetalicRoughnessRTV = resourceManager->Get<RenderTargetView>(L"Metalic_Roughness").lock();
	m_GBuffer = resourceManager->Get<RenderTargetView>(L"GBuffer").lock();


	const uint32_t width = m_Device.lock()->GetWndWidth();
	const uint32_t height = m_Device.lock()->GetWndHeight();
	m_NormalCopyRTV = resourceManager->Create<RenderTargetView>(L"NormalCopyRTV", RenderTargetViewType::OffScreen, width, height).lock();

	m_QuadVB = resourceManager->Get<VertexBuffer>(L"Quad_VB");
	m_QuadIB = resourceManager->Get<IndexBuffer>(L"Quad_IB");
	m_QuadVS = resourceManager->Get<VertexShader>(L"Quad");
	m_QuadPS = resourceManager->Get<PixelShader>(L"CopyTex");


	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = resourceManager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = resourceManager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicRoughnessSRV = resourceManager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_AmbientOcclusionSRV = resourceManager->Get<ShaderResourceView>(L"AO").lock();
	m_EmissiveSRV = resourceManager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = resourceManager->Get<ShaderResourceView>(L"GBuffer").lock();
	m_NormalCopySRV = resourceManager->Create<ShaderResourceView>(L"NormalCopySRV", m_NormalCopyRTV.lock()).lock();


	//데칼 최대 1천개까지 가능
	for (int i = 0; i < 1000; i++)
	{
		InstanceDecalData temp;
		m_InstanceDatas.push_back(temp);
	}
	m_InstanceBuffer = m_ResourceManager.lock()->Create<VertexBuffer>(L"InstanceBuffer", m_InstanceDatas, true);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_Device.lock()->Context()->Map(m_InstanceBuffer.lock()->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	InstanceDecalData* dataView = reinterpret_cast<InstanceDecalData*>(mappedData.pData);
	dataView = nullptr;
	m_Device.lock()->Context()->Unmap(m_InstanceBuffer.lock()->Get(), 0);
	m_InstanceDatas.clear();

	m_DecalVB = m_ResourceManager.lock()->Get<VertexBuffer>(L"Decal_VB");
	m_DecalIB = m_ResourceManager.lock()->Get<IndexBuffer>(L"Decal_IB");

	m_DecalVS = m_ResourceManager.lock()->Get<VertexShader>(L"DecalVS");
	m_DecalPS = m_ResourceManager.lock()->Get<PixelShader>(L"DecalPS");
}

DecalPass::~DecalPass()
{

}

void DecalPass::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
	std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearClamp").lock();
	std::shared_ptr<Sampler> linearWrap = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	//현재 normal을 복사해 decal을 그려낼때 필요한 정보를 만들어내자
	//{

	//	Device->UnBindSRV();
	//	FLOAT Black[4] = { 0.f,0.f,0.f,1.f };
	//	Device->Context()->ClearRenderTargetView(m_NormalCopyRTV.lock()->Get(), Black);

	//	Device->Context()->OMSetRenderTargets(1, m_NormalCopyRTV.lock()->GetAddress(), nullptr);
	//	Device->BindVS(m_QuadVS.lock());
	//	Device->Context()->RSSetState(m_ResourceManager.lock()->Get<RenderState>(L"Solid").lock()->Get());

	//	std::shared_ptr<VertexBuffer> vb = m_QuadVB.lock();
	//	std::shared_ptr<IndexBuffer> ib = m_QuadIB.lock();
	//	m_Device.lock()->Context()->IASetVertexBuffers(0, 1, vb->GetAddress(), vb->Size(), vb->Offset());
	//	m_Device.lock()->Context()->IASetIndexBuffer(ib->Get(), DXGI_FORMAT_R32_UINT, 0);
	//	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	Device->Context()->PSSetShader(m_QuadPS.lock()->GetPS(), nullptr, 0);
	//	Device->Context()->PSSetShaderResources(0, 1, m_NormalSRV.lock()->GetAddress());
	//	Device->Context()->PSSetSamplers(static_cast<UINT>(Slot_S::Linear), 1, linearWrap->GetAddress());

	//	Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);
	//}



	//instance buffer update
	std::map<std::string, std::vector<decal::Info>>& curDecals = m_DecalManager->GetDecals();
	for (auto& decals : curDecals)
	{
		for (auto& decal : decals.second)
		{
			InstanceDecalData temp;
			temp.world = decal.WorldTransform.Transpose();
			temp.worldInverse = temp.world.Invert();//회전 역행렬

			m_InstanceDatas.push_back(temp);
		}
	}

	if (!m_InstanceDatas.empty())
	{

		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_Device.lock()->Context()->Map(m_InstanceBuffer.lock()->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstanceDecalData* dataView = reinterpret_cast<InstanceDecalData*>(mappedData.pData);

		for (int i = 0; i < m_InstanceDatas.size(); i++)
		{
			dataView[i] = m_InstanceDatas[i];
		}
		m_Device.lock()->Context()->Unmap(m_InstanceBuffer.lock()->Get(), 0);
	}


	//bind
	Device->UnBindSRV();
	Device->Context()->VSSetConstantBuffers(0, 1, CameraCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(0, 1, CameraCB->GetAddress());

	//set rtv,dsv
	std::vector<ID3D11RenderTargetView*> RTVs;
	RTVs.push_back(m_GBuffer.lock()->Get());
	//RTVs.push_back(m_AlbedoRTV.lock()->Get());
	//RTVs.push_back(m_NormalRTV.lock()->Get());
	Device->Context()->OMSetRenderTargets(RTVs.size(), RTVs.data(), m_DepthStencilView.lock()->Get());


	//set vb(instance buffer)
	UINT strides[2];
	strides[0] = sizeof(DecalVertex);
	strides[1] = sizeof(InstanceDecalData);
	UINT offsets[2];
	offsets[0] = 0;
	offsets[1] = 0;

	std::vector<ID3D11Buffer*> bufferPointers[2];
	bufferPointers->push_back(m_DecalVB.lock()->Get());
	bufferPointers->push_back(m_InstanceBuffer.lock()->Get());

	m_Device.lock()->Context()->IASetVertexBuffers(0, 2, bufferPointers->data(), strides, offsets);

	//set inputlayout,ib
	m_Device.lock()->Context()->IASetInputLayout(m_DecalVS.lock()->InputLayout());
	m_Device.lock()->Context()->IASetIndexBuffer(m_DecalIB.lock()->Get(), DXGI_FORMAT_R32_UINT, 0);

	//set primitive
	m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//set vs, ps
	m_Device.lock()->Context()->VSSetShader(m_DecalVS.lock()->GetVS(), nullptr, 0);
	m_Device.lock()->Context()->PSSetShader(m_DecalPS.lock()->GetPS(), nullptr, 0);

	//set sampler
	m_Device.lock()->Context()->PSSetSamplers(0, 1, linear->GetAddress());

	//set cb
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());

	//set srv
	m_Device.lock()->Context()->PSSetShaderResources(0, 1, m_PositionSRV.lock()->GetAddress());
	//m_Device.lock()->Context()->PSSetShaderResources(1, 1, m_NormalCopySRV.lock()->GetAddress());
	m_Device.lock()->Context()->PSSetShaderResources(1, 1, m_NormalSRV.lock()->GetAddress());

	auto blendState = m_ResourceManager.lock()->Get<BlendState>(L"AlphaBlend");

	Device->Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	m_Device.lock()->Context()->OMSetBlendState(blendState.lock()->GetState().Get(), nullptr, 0xFFFFFFFF);

	std::shared_ptr<DepthStencilState> depth = m_ResourceManager.lock()->Get<DepthStencilState>(L"NoDepthWrites").lock();
	Device->Context()->OMSetDepthStencilState(depth->GetState().Get(), 1);

	//render
	int offset = 0;
	for (auto& decals : curDecals)
	{
		std::wstring wDecalName;
		wDecalName.assign(decals.first.begin(), decals.first.end());
		std::weak_ptr<ShaderResourceView> decaltex = m_ResourceManager.lock()->Get<ShaderResourceView>(wDecalName);
		if (decaltex.lock() == nullptr)
		{
			decaltex = m_ResourceManager.lock()->Get<ShaderResourceView>(L"base.png");
		}
		m_Device.lock()->Context()->PSSetShaderResources(2, 1, decaltex.lock()->GetAddress());

		std::weak_ptr<ShaderResourceView> decalnormaltex = m_DecalManager->GetNormalDecal(decals.first);
		if (decalnormaltex.lock() == nullptr)
		{
			decalnormaltex = m_ResourceManager.lock()->Get<ShaderResourceView>(L"normalbase.png");
		}
		//m_Device.lock()->Context()->PSSetShaderResources(3, 1, decalnormaltex.lock()->GetAddress());

		auto& curDecal = decals.second;
		m_Device.lock()->Context()->DrawIndexedInstanced(DecalVolume::Index::count, curDecal.size(), 0, 0, offset);
		offset += curDecal.size();
	}

	Device->Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	Device->Context()->OMSetDepthStencilState(nullptr, 1);

	m_DecalManager->ClearDecals();
	m_InstanceDatas.clear();
}

void DecalPass::OnResize()
{
	std::shared_ptr<ResourceManager> manager = m_ResourceManager.lock();

	m_DepthStencilView = manager->Get<DepthStencilView>(L"DSV_Deferred").lock();

	m_AlbedoRTV = manager->Get<RenderTargetView>(L"Albedo").lock();
	m_NormalRTV = manager->Get<RenderTargetView>(L"Normal").lock();
	m_PositionRTV = manager->Get<RenderTargetView>(L"Position").lock();
	m_DepthRTV = manager->Get<RenderTargetView>(L"Depth").lock();
	m_MetalicRoughnessRTV = manager->Get<RenderTargetView>(L"Metalic_Roughness").lock();
	m_AORTV = manager->Get<RenderTargetView>(L"AO").lock();
	m_EmissiveRTV = manager->Get<RenderTargetView>(L"Emissive").lock();
	m_LightMapRTV = manager->Get<RenderTargetView>(L"LightMap").lock();
	m_GBuffer = manager->Get<RenderTargetView>(L"GBuffer").lock();

	m_NormalCopyRTV.lock()->OnResize();

	m_ResourceManager.lock()->Erase<ShaderResourceView>(L"NormalCopySRV");
	m_NormalCopySRV = m_ResourceManager.lock()->Create<ShaderResourceView>(L"NormalCopySRV", m_NormalCopyRTV.lock()).lock();

	m_AlbedoSRV = manager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = manager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = manager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = manager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicRoughnessSRV = manager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_AmbientOcclusionSRV = manager->Get<ShaderResourceView>(L"AO").lock();
	m_EmissiveSRV = manager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = manager->Get<ShaderResourceView>(L"GBuffer").lock();
	m_LightMapSRV = manager->Get<ShaderResourceView>(L"LightMap").lock();
}
