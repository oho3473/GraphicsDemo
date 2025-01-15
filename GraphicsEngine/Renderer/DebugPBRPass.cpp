#include "DebugPBRPass.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelData.h"

void DebugPBRPass::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager, const std::shared_ptr<LightManager>& lightManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;
	m_LightManager = lightManager;

	m_DepthStencilView = resourceManager->Get<DepthStencilView>(L"DSV_DebugPBR").lock();

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_MetalicSRV = resourceManager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_RoughnessSRV = resourceManager->Get<ShaderResourceView>(L"AO").lock();
	m_IrrandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3DiffuseHDR.dds").lock();
	m_RandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3SpecularHDR.dds").lock();

	m_MeshPS = resourceManager->Get<PixelShader>(L"DebugPBR");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base").lock();
	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning").lock();

	m_FresnelRTV = resourceManager->Get<RenderTargetView>(L"Fresnel").lock();
	m_DistributeRTV = resourceManager->Get<RenderTargetView>(L"Distribute").lock();
	m_GeometryRTV = resourceManager->Get<RenderTargetView>(L"Geometry").lock();
	m_SpecularRTV = resourceManager->Get<RenderTargetView>(L"Specular").lock();
	m_DiffuseRTV = resourceManager->Get<RenderTargetView>(L"Diffuse").lock();

	m_TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	m_SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	m_CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera");

}

void DebugPBRPass::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	// Bind Common Resources
	{
		Device->UnBindSRV();
		std::vector<ID3D11RenderTargetView*> RTVs;

		int GBufferSize = 5;
		RTVs.reserve(GBufferSize);

		RTVs.push_back(m_FresnelRTV.lock()->Get());
		RTVs.push_back(m_DistributeRTV.lock()->Get());
		RTVs.push_back(m_GeometryRTV.lock()->Get());
		RTVs.push_back(m_SpecularRTV.lock()->Get());
		RTVs.push_back(m_DiffuseRTV.lock()->Get());
		Device->Context()->OMSetRenderTargets(GBufferSize, RTVs.data(), m_DepthStencilView.lock()->Get());

		Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);

		std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
		std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
		std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
		std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
		std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());
		Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());
		Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());
		Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());
	}

	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> useEditMaterial = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"EditMaterial").lock();
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> editAlbedo = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"EditAlbedo").lock();
	Device->Context()->PSSetConstantBuffers(5, 1, editAlbedo->GetAddress());
	Device->Context()->PSSetConstantBuffers(6, 1, useEditMaterial->GetAddress());


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
}

void DebugPBRPass::OnResize()
{
	std::shared_ptr<ResourceManager> resourceManager = m_ResourceManager.lock();


	m_DepthStencilView = resourceManager->Get<DepthStencilView>(L"DSV_DebugPBR").lock();

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_MetalicSRV = resourceManager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_RoughnessSRV = resourceManager->Get<ShaderResourceView>(L"AO").lock();
	m_IrrandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3DiffuseHDR.dds").lock();
	m_RandianceSRV = resourceManager->Get<ShaderResourceView>(L"MyCube3SpecularHDR.dds").lock();

	m_FresnelRTV = resourceManager->Get<RenderTargetView>(L"Fresnel").lock();
	m_DistributeRTV = resourceManager->Get<RenderTargetView>(L"Distribute").lock();
	m_GeometryRTV = resourceManager->Get<RenderTargetView>(L"Geometry").lock();
	m_SpecularRTV = resourceManager->Get<RenderTargetView>(L"Specular").lock();
	m_DiffuseRTV = resourceManager->Get<RenderTargetView>(L"Diffuse").lock();
}
