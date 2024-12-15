#include "pch.h"
#include "OverDrawPass.h"
#include "ResourceManager.h"
#include "StaticData.h"
#include "Slot.h"

#include "ModelData.h"
#include "Mesh.h"
#include "Material.h"
#include "Device.h"
#include "BlendState.h"
#include "DepthStencilState.h"

OverDrawPass::OverDrawPass(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager) : RenderPass()
{
	m_ResourceManager = manager;
	m_Device = device;

	m_meshPS = manager->Get<PixelShader>(L"OverDrawPS");
	m_Gbuffer = manager->Get<RenderTargetView>(L"GBuffer").lock();
	m_Normal = manager->Get<RenderTargetView>(L"Normal").lock();

	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base");
}

OverDrawPass::~OverDrawPass()
{

}

void OverDrawPass::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	std::shared_ptr<RenderTargetView> rtv = m_ResourceManager.lock()->Get<RenderTargetView>(L"GBuffer").lock();
	std::shared_ptr<RenderTargetView> normal = m_ResourceManager.lock()->Get<RenderTargetView>(L"Normal").lock();

	std::shared_ptr<DepthStencilView> dsv = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Main").lock();
	m_Device.lock()->Context()->ClearDepthStencilView(dsv->Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	Device->UnBindSRV();
	Device->Context()->RSSetState(resourcemanager->Get<RenderState>(L"Solid").lock()->Get());

	std::vector<ID3D11RenderTargetView*> RTVs;
	int GBufferSize = 2;//최대 8개 밖에 안됨
	RTVs.reserve(GBufferSize);
	RTVs.push_back(m_Gbuffer.lock()->Get());
	RTVs.push_back(m_Normal.lock()->Get());

	Device->Context()->OMSetRenderTargets(GBufferSize, RTVs.data(), dsv->Get());

	Device->Context()->PSSetShader(m_meshPS.lock()->GetPS(), nullptr, 0);
	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

	std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
	std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
	std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());

	Device->Context()->PSSetConstantBuffers(0, 1, CameraCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(1, 1, MaterialCB->GetAddress());

	for (const auto& curData : m_RenderList)
	{
		if (!curData->isOverDraw)
		{
			continue;
		}

		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(curData->FBX).lock();
		if (curModel != nullptr)
		{

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

				if (!curModel->m_Materials.empty())
				{
					std::shared_ptr<ConstantBuffer<MaterialData>> curMaterialData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
					std::shared_ptr<Material> curMaterial = curModel->m_Materials[mesh->m_material];

					if (curMaterial != nullptr)
					{
						MaterialData data = curMaterial->m_Data;
						curMaterialData->Update(data);

						Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

						Device->BindMaterialSRV(curMaterial);
					}
				}

				Device->Context()->DrawIndexed(mesh->IBCount(), 0, 0);
			}
		}
	}
}

void OverDrawPass::OnResize()
{
	std::shared_ptr<ResourceManager> manager = m_ResourceManager.lock();

	m_Gbuffer = manager->Get<RenderTargetView>(L"GBuffer").lock();
	m_Normal = manager->Get<RenderTargetView>(L"Normal").lock();
}
