#include "pch.h"
#include "DeferredGeometryPass.h"

#include "ResourceManager.h"
#include "Desc.h"
#include "Material.h"
#include "Mesh.h"
#include "Slot.h"

#include <vector>


void DeferredGeometryPass::Initialize(const std::shared_ptr<Device>& device,
	const std::shared_ptr<ResourceManager>& resourceManager,
	const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	m_Device = device;
	m_ResourceManager = resourceManager;

	m_View = view;
	m_Proj = proj;

	m_DepthStencilView = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Deferred").lock();

	m_AlbedoRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Albedo").lock();
	m_NormalRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Normal").lock();
	m_PositionRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Position").lock();
	m_DepthRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Depth").lock();
	m_MetalicRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Metalic").lock();
	m_RoughnessRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Roughness").lock();
	m_AORTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"AO").lock();
	m_EmissiveRTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"Emissive").lock();

	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base").lock();
	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning").lock();
	m_GeometryPS = m_ResourceManager.lock()->Get<PixelShader>(L"MeshDeferredGeometry").lock();

	m_TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	m_SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	m_CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera");
}

void DeferredGeometryPass::Render(const std::shared_ptr<RenderData>& model)
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"Linear").lock();

	// Bind Common Resources
	{
		Device->UnBindSRV();
		std::vector<ID3D11RenderTargetView*> RTVs;

		int GBufferSize = static_cast<UINT>(Slot_T::End) - 1;
		RTVs.reserve(GBufferSize);

		Device->Context()->OMSetDepthStencilState(m_DSS_Null.Get(), 0);
		Device->Context()->OMSetBlendState(m_BS_Null.Get(), nullptr, 0xFFFFFFFF);

		RTVs.push_back(m_AlbedoRTV->Get());
		RTVs.push_back(m_NormalRTV->Get());
		RTVs.push_back(m_PositionRTV->Get());
		RTVs.push_back(m_DepthRTV->Get());
		RTVs.push_back(m_MetalicRTV->Get());
		RTVs.push_back(m_RoughnessRTV->Get());
		RTVs.push_back(m_AORTV->Get());
		RTVs.push_back(m_EmissiveRTV->Get());
		Device->Context()->OMSetRenderTargets(GBufferSize, RTVs.data(), m_DepthStencilView->Get());

		Device->Context()->PSSetShader(m_GeometryPS->GetPS(), nullptr, 0);
		Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, m_TransformCB->GetAddress());
		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, m_SkeletalCB->GetAddress());
		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, m_CameraCB.lock()->GetAddress());
		Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, m_CameraCB.lock()->GetAddress());
	}

	// Mesh Update & Bind & Draw
	{
		std::shared_ptr<RenderData> curData = model;
		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(curData->FBX).lock();

		if (curModel != nullptr)
		{

			int materialindex = 0; //mesh의 숫자와 동일

			for (const auto& mesh : curModel->m_Meshes)
			{
				Device->BindMeshBuffer(mesh);

				// Static Mesh Data Update & Bind
				if (!mesh->IsSkinned())
				{

					Device->BindVS(m_StaticMeshVS);

					// CB Update
					TransformData renew;
					XMStoreFloat4x4(&renew.local, XMMatrixTranspose(curData->world));
					XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));
					m_TransformCB->Update(renew);	// == Bind
				}
				// Skeletal Mesh Update & Bind
				else
				{
					Device->BindVS(m_SkeletalMeshVS);

					std::shared_ptr<SkinnedMesh> curMesh = std::dynamic_pointer_cast<SkinnedMesh>(mesh);

					// CB Update
					TransformData renew;
					XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));
					renew.local = curMesh->m_node.lock()->m_World;
					m_TransformCB->Update(renew);	// == Bind

					MatrixPallete matrixPallete = *(curMesh->Matrix_Pallete);
					m_SkeletalCB->Update(matrixPallete);

					//m_Camera = Create<ConstantBuffer<CameraData>>(L"Camera", BufferDESC::Constant::DefaultCamera);
					//m_Device.lock()->Context()->VSSetConstantBuffers(0, 1, (m_Camera.lock()->GetAddress()));

				}

				// 텍스처와 샘플러를 셰이더에 바인딩
				if (!curModel->m_Materials.empty())
				{
					std::shared_ptr<ConstantBuffer<MaterialData>> curData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();

					std::shared_ptr<Material> curMaterial = curModel->m_Materials[materialindex];
					MaterialData curMaterialData = curMaterial->m_Data;
					curData->Update(curMaterialData);

					Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

					Device->BindMaterialSRV(curMaterial);
				}

				Device->Context()->DrawIndexed(mesh->IBCount(), 0, 0);

				materialindex++;

			}

			//렌더타겟 해제해줘야지 srv도 해제
			Device->Context()->OMSetRenderTargets(0, nullptr, nullptr);
		}
	}
}
