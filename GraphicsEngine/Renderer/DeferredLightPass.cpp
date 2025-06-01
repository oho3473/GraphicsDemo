#include "pch.h"
#include "DeferredLightPass.h"
#include "DepthStencilState.h"

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
	m_MetalicRTV = resourceManager->Get<RenderTargetView>(L"Metalic").lock();
	m_RoughnessRTV = resourceManager->Get<RenderTargetView>(L"Roughness").lock();
	m_EmissiveRTV = resourceManager->Get<RenderTargetView>(L"Emissive").lock();
	m_LightMapRTV = resourceManager->Get<RenderTargetView>(L"CubeMapSRV").lock();

	m_QuadVB = resourceManager->Get<VertexBuffer>(L"Quad_VB");
	m_QuadIB = resourceManager->Get<IndexBuffer>(L"Quad_IB");
	m_QuadVS = resourceManager->Get<VertexShader>(L"Quad");

	m_Deferred = resourceManager->Get<PixelShader>(L"MeshDeferredLight");

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = resourceManager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = resourceManager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicSRV = resourceManager->Get<ShaderResourceView>(L"Metalic").lock();
	m_RoughnessSRV = resourceManager->Get<ShaderResourceView>(L"AO").lock();
	m_EmissiveSRV = resourceManager->Get<ShaderResourceView>(L"Emissive").lock();

	m_IrrandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3DiffuseHDR.dds").lock();
	m_RandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3SpecularHDR.dds").lock();

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
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> useIBL = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"useIBL").lock();

	std::shared_ptr<DepthStencilState> depthstencilstate = m_ResourceManager.lock()->Get<DepthStencilState>(L"AbleStencil").lock();

	UnBindResource();

	Device->Context()->OMSetDepthStencilState(depthstencilstate->GetState().Get(), 1);

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

	Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::IBL), 1, useIBL->GetAddress());


	//Save GBuffer texture
	{
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
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Metalic), 1, m_MetalicSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Roughness), 1, m_RoughnessSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Emissive), 1, m_EmissiveSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::LightMap), 1, m_LightMapSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Irradiance), 1, m_IrrandianceSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::Radiance), 1, m_RandianceSRV.lock()->GetAddress());
		Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::LUT), 1, m_LUT.lock()->GetAddress());

		Device->Context()->PSSetSamplers(static_cast<UINT>(Slot_S::Linear), 1, linear->GetAddress());

		Device->Context()->OMSetRenderTargets(1, m_GbufferRTV.lock()->GetAddress(), m_DepthStencilView.lock()->Get());

		Device->Context()->DrawIndexed(Quad::Index::count, 0, 0);
		Device->Context()->OMSetDepthStencilState(nullptr, 1);

	}

	UnBindResource();
}

void DeferredLightPass::OnResize()
{
	std::shared_ptr<ResourceManager> manager = m_ResourceManager.lock();

	m_DepthStencilView = manager->Get<DepthStencilView>(L"DSV_Deferred").lock();

	m_AlbedoRTV = manager->Get<RenderTargetView>(L"Albedo").lock();
	m_NormalRTV = manager->Get<RenderTargetView>(L"Normal").lock();
	m_PositionRTV = manager->Get<RenderTargetView>(L"Position").lock();
	m_DepthRTV = manager->Get<RenderTargetView>(L"Depth").lock();
	m_MetalicRTV = manager->Get<RenderTargetView>(L"Metalic").lock();
	m_RoughnessRTV = manager->Get<RenderTargetView>(L"Roughness").lock();
	m_EmissiveRTV = manager->Get<RenderTargetView>(L"Emissive").lock();
	m_LightMapRTV = manager->Get<RenderTargetView>(L"LightMap").lock();

	m_AlbedoSRV = manager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = manager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = manager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = manager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicSRV = manager->Get<ShaderResourceView>(L"Metalic").lock();
	m_RoughnessSRV = manager->Get<ShaderResourceView>(L"Roughness").lock();
	m_EmissiveSRV = manager->Get<ShaderResourceView>(L"Emissive").lock();

	m_LightMapSRV = manager->Get<ShaderResourceView>(L"LightMap").lock();
	m_IrrandianceSRV = manager->Get<ShaderResourceView>(L"MyCube3DiffuseHDR.dds").lock();
	m_RandianceSRV = manager->Get<ShaderResourceView>(L"MyCube3SpecularHDR.dds").lock();
	m_LUT = manager->Get<ShaderResourceView>(L"MyCube3Brdf.dds").lock();


	m_FresnelRTV = manager->Get<RenderTargetView>(L"Fresnel").lock();
	m_DistributeRTV = manager->Get<RenderTargetView>(L"Distribute").lock();
	m_GeometryRTV = manager->Get<RenderTargetView>(L"Geometry").lock();
	m_SpecularRTV = manager->Get<RenderTargetView>(L"Specular").lock();
	m_DiffuseRTV = manager->Get<RenderTargetView>(L"Diffuse").lock();
	m_GbufferRTV = manager->Get<RenderTargetView>(L"GBuffer").lock();


	m_PBRs.clear();
	m_PBRs.push_back(m_GbufferRTV.lock()->Get());
	m_PBRs.push_back(m_FresnelRTV.lock()->Get());
	m_PBRs.push_back(m_DistributeRTV.lock()->Get());
	m_PBRs.push_back(m_GeometryRTV.lock()->Get());
	m_PBRs.push_back(m_SpecularRTV.lock()->Get());
	m_PBRs.push_back(m_DiffuseRTV.lock()->Get());

}

void DeferredLightPass::ChangeCubeTex(const std::wstring name)
{
	std::shared_ptr<ResourceManager> resourceManager = m_ResourceManager.lock();

	//TODO: 바꾼 cubemap에 맞는 diffuse specular로 변경해줘야함
	m_RandianceSRV = resourceManager->Get<ShaderResourceView>(name).lock();
	m_IrrandianceSRV = resourceManager->Get<ShaderResourceView>(name).lock();
}

