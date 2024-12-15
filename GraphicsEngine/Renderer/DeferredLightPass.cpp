#include "pch.h"
#include "DeferredLightPass.h"

#include "Device.h"
#include "ResourceManager.h"

#include"StaticData.h"
#include "Slot.h"

DeferredLightPass::~DeferredLightPass()
{

}

void DeferredLightPass::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,
	const std::shared_ptr<LightManager>& lightManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;
	m_LightManager = lightManager;

	m_DepthStencilView = resourceManager->Get<DepthStencilView>(L"DSV_Deferred").lock();

	m_AlbedoRTV = resourceManager->Get<RenderTargetView>(L"Albedo").lock();
	m_NormalRTV = resourceManager->Get<RenderTargetView>(L"Normal").lock();
	m_PositionRTV = resourceManager->Get<RenderTargetView>(L"Position").lock();
	m_DepthRTV = resourceManager->Get<RenderTargetView>(L"Depth").lock();
	m_MetalicRoughnessRTV = resourceManager->Get<RenderTargetView>(L"Metalic_Roughness").lock();
	m_AORTV = resourceManager->Get<RenderTargetView>(L"AO").lock();
	m_EmissiveRTV = resourceManager->Get<RenderTargetView>(L"Emissive").lock();
	m_LightMapRTV = resourceManager->Get<RenderTargetView>(L"LightMap").lock();

	m_StaticMeshVS = resourceManager->Get<VertexShader>(L"Base").lock();
	m_StaticMeshVS = resourceManager->Get<VertexShader>(L"Skinning").lock();
	m_GeometryPS = resourceManager->Get<PixelShader>(L"MeshDeferredGeometry").lock();

	m_QuadVB = resourceManager->Get<VertexBuffer>(L"Quad_VB");
	m_QuadIB = resourceManager->Get<IndexBuffer>(L"Quad_IB");
	m_QuadVS = resourceManager->Get<VertexShader>(L"Quad");
	m_QuadPS = resourceManager->Get<PixelShader>(L"Quad");

	m_Deferred = resourceManager->Get<PixelShader>(L"MeshDeferredLight");

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = resourceManager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = resourceManager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicRoughnessSRV = resourceManager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_AmbientOcclusionSRV = resourceManager->Get<ShaderResourceView>(L"AO").lock();
	m_EmissiveSRV = resourceManager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = resourceManager->Get<ShaderResourceView>(L"GBuffer").lock();

	m_SkeletalMeshVS = resourceManager->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = resourceManager->Get<VertexShader>(L"Base");
	m_MeshPS = resourceManager->Get<PixelShader>(L"MeshDeferredGeometry");
}


void DeferredLightPass::Render()
{

	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();
	std::shared_ptr<VertexBuffer> vb = m_QuadVB.lock();
	std::shared_ptr<IndexBuffer> ib = m_QuadIB.lock();


	std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
	std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
	std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());
	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());

	//Save GBuffer texture
	{
		std::shared_ptr<RenderTargetView> rtv = resourcemanager->Get<RenderTargetView>(L"GBuffer").lock();
		std::shared_ptr<DepthStencilView> dsv = resourcemanager->Get<DepthStencilView>(L"DSV_Deferred").lock();

		Device->BindVS(m_QuadVS.lock());
		Device->Context()->PSSetShader(m_Deferred.lock()->GetPS(), nullptr, 0);

		Device->Context()->RSSetState(resourcemanager->Get<RenderState>(L"Solid").lock()->Get());
		m_Device.lock()->Context()->IASetVertexBuffers(0, 1, vb->GetAddress(), vb->Size(), vb->Offset());
		m_Device.lock()->Context()->IASetIndexBuffer(ib->Get(), DXGI_FORMAT_R32_UINT, 0);

		m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Albedo), 1, m_AlbedoSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Normal), 1, m_NormalSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Position), 1, m_PositionSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Depth), 1, m_DepthSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Metalic), 1, m_MetalicRoughnessSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::AO), 1, m_AmbientOcclusionSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Emissive), 1, m_EmissiveSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::LightMap), 1, m_LightMapSRV.lock()->GetAddress());

		Device->Context()->PSSetSamplers(static_cast<UINT>(Slot_S::Linear), 1, linear->GetAddress());

		Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), nullptr);

		Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);
	}
}

void DeferredLightPass::OnResize()
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

