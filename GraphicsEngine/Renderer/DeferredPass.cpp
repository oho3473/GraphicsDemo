#include "pch.h"
#include "DeferredPass.h"
#include "ResourceManager.h"
#include "ShaderResourceView.h"
#include "RenderTargetView.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "ModelData.h"
#include "Mesh.h"
#include "Material.h"

#include "Slot.h"
#include "StaticData.h"
//#include "DebugDrawManager.h"
#include "LightManager.h"


DeferredPass::~DeferredPass()
{

}

void DeferredPass::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,
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
	m_LightMapRTV = resourceManager->Get<RenderTargetView>(L"LightMap").lock();

	m_GeometryPS = resourceManager->Get<PixelShader>(L"MeshDeferredGeometry").lock();

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = resourceManager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = resourceManager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicSRV = resourceManager->Get<ShaderResourceView>(L"Metalic").lock();
	m_RoughnessSRV = resourceManager->Get<ShaderResourceView>(L"Roughness").lock();
	m_EmissiveSRV = resourceManager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = resourceManager->Get<ShaderResourceView>(L"GBuffer").lock();

	m_SkeletalMeshVS = resourceManager->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = resourceManager->Get<VertexShader>(L"Base");
	m_MeshPS = resourceManager->Get<PixelShader>(L"MeshDeferredGeometry");
}

void DeferredPass::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	// Bind Common Resources
	Device->UnBindSRV();
	std::vector<ID3D11RenderTargetView*> RTVs;
	int GBufferSize = 8;//최대 8개 밖에 안됨
	RTVs.reserve(GBufferSize);

	RTVs.push_back(m_AlbedoRTV.lock()->Get());
	RTVs.push_back(m_NormalRTV.lock()->Get());
	RTVs.push_back(m_PositionRTV.lock()->Get());
	RTVs.push_back(m_DepthRTV.lock()->Get());
	RTVs.push_back(m_MetalicRTV.lock()->Get());
	RTVs.push_back(m_RoughnessRTV.lock()->Get());
	RTVs.push_back(m_LightMapRTV.lock()->Get());
	RTVs.push_back(m_EmissiveRTV.lock()->Get());


	Device->Context()->OMSetRenderTargets(GBufferSize, RTVs.data(), m_DepthStencilView.lock()->Get());

	Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);
	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());


	std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
	std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
	std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();

	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> useEditMaterial = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"EditMaterial").lock();
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> editAlbedo = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"EditAlbedo").lock();

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());

	Device->Context()->PSSetConstantBuffers(1, 1, useEditMaterial->GetAddress());
	Device->Context()->PSSetConstantBuffers(0, 1, editAlbedo->GetAddress());



	for (const auto& curData : m_RenderList)
	{
		bool isTranparency = false;

		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(curData->FBX).lock();

		if (curModel != nullptr)
		{
			if (!curModel->m_Materials.empty())
			{
				for (auto material : curModel->m_Materials)
				{
					if (material->m_OpacitySRV.lock() != nullptr)
					{
						isTranparency = true;
						break;
					}
				}
			}

			if (isTranparency)
			{
				continue;
			}

			for (const auto& mesh : curModel->m_Meshes)
			{
				Device->BindMeshBuffer(mesh);

				// Static Mesh Data Update & Bind
				if (!mesh->IsSkinned())
				{
					Device->BindVS(m_StaticMeshVS.lock());

					// CB Update
					std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();

					TransformData renew;
					XMStoreFloat4x4(&renew.local, XMMatrixTranspose(curData->world));
					XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));
					XMStoreFloat4x4(&renew.localInverse, (curData->world.Invert()));//전치 해주지말자 회전의 역행렬은 전치행렬임
					XMStoreFloat4x4(&renew.worldInverse, (curData->world.Invert()));//전치 해주지말자 회전의 역행렬은 전치행렬임
					position->Update(renew);	// == Bind
				}
				else
				{
					Device->BindVS(m_SkeletalMeshVS.lock());

					std::shared_ptr<SkinnedMesh> curMesh = std::dynamic_pointer_cast<SkinnedMesh>(mesh);

					// CB Update
					std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();

					TransformData renew;
					XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));

					if (curMesh->m_node.lock() != nullptr)
					{
						renew.local = curMesh->m_node.lock()->m_World;
					}
					else
					{
						renew.local = DirectX::SimpleMath::Matrix::Identity;
					}

					XMStoreFloat4x4(&renew.localInverse, (renew.local.Invert()));
					XMStoreFloat4x4(&renew.worldInverse, (renew.world.Invert()));

					position->Update(renew);
					std::shared_ptr<ConstantBuffer<MatrixPallete>> pallete;
					if (!curData->FBX.empty() && curData->isPlay)
					{
						std::wstring id = std::to_wstring(curData->EntityID);
						pallete = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(id).lock();
					}
					else
					{
						pallete = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
					}
					pallete->Update();
					Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, pallete->GetAddress());

				}
				// 텍스처와 샘플러를 셰이더에 바인딩
				if (!curModel->m_Materials.empty())
				{
					std::shared_ptr<ConstantBuffer<MaterialData>> curMaterialData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
					std::shared_ptr<Material> curMaterial = curModel->m_Materials[mesh->m_material];

					if (curMaterial != nullptr)
					{
						MaterialData data = curMaterial->m_Data;
						data.lightmapdata.x = curData->offset.x;
						data.lightmapdata.y = curData->offset.y;
						data.lightmapdata.z = static_cast<float>(curData->lightmapindex);
						data.lightmapdata.w = 1; //curData->scale;
						data.lightmaptiling = curData->tiling;
						if (data.lightmaptiling.x != 0 || data.lightmaptiling.y != 0)
						{
							data.useNEOL.w = 1;

							std::shared_ptr<ShaderResourceView> lightmap = m_LightManager.lock()->GetLightMap(curData->lightmapindex).lock();
							if (lightmap != nullptr)
							{
								Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::LightMap), 1, lightmap->GetAddress());
							}
						}
						curMaterialData->Update(data);

						Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

						Device->BindMaterialSRV(curMaterial);
					}
				}

				useEditMaterial->Update(curData->metalicRoughness);
				editAlbedo->Update(curData->albedo);

				Device->Context()->DrawIndexed(mesh->IBCount(), 0, 0);
			}
		}
	}

	//렌더타겟 해제해줘야지
	Device->Context()->OMSetRenderTargets(0, nullptr, nullptr);
}

void DeferredPass::OnResize()
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
	m_GBufferSRV = manager->Get<ShaderResourceView>(L"GBuffer").lock();
	m_LightMapSRV = manager->Get<ShaderResourceView>(L"LightMap").lock();
}
