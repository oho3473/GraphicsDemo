#include "pch.h"
#include "RimLight.h"
#include "ResourceManager.h"
#include "StaticData.h"
#include "Slot.h"

#include "ModelData.h"
#include "Mesh.h"
#include "Material.h"
#include "Device.h"
#include "BlendState.h"
#include "DepthStencilState.h"

RimLight::RimLight(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager) : RenderPass()
{
	m_ResourceManager = manager;
	m_Device = device;

	m_RimLightPS = manager->Get<PixelShader>(L"RimLight");
	m_GbufferSRV = manager->Get<ShaderResourceView>(L"GBuffer").lock();

	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base");

}

RimLight::~RimLight()
{

}

void RimLight::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	std::shared_ptr<RenderTargetView> rtv = m_ResourceManager.lock()->Get<RenderTargetView>(L"GBuffer").lock();
	std::shared_ptr<DepthStencilView> dsv = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Main").lock();
	std::shared_ptr<DepthStencilView> deferredDsv = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Deferred").lock();


	Device->UnBindSRV();
	Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), dsv->Get());
	Device->Context()->RSSetState(resourcemanager->Get<RenderState>(L"Solid").lock()->Get());

	Device->Context()->PSSetShader(m_RimLightPS.lock()->GetPS(), nullptr, 0);
	Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

	std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
	std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
	std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
	std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
	std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();
	std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> color = m_ResourceManager.lock()->Get<ConstantBuffer<DirectX::XMFLOAT4>>(L"Color").lock();

	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Transform), 1, TransformCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Material), 1, MaterialCB->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::LightArray), 1, light->GetAddress());
	Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, SkeletalCB->GetAddress());

	Device->Context()->PSSetConstantBuffers(0, 1, CameraCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(1, 1, MaterialCB->GetAddress());
	Device->Context()->PSSetConstantBuffers(2, 1, color->GetAddress());

	for (const auto& curData : m_RenderList)
	{
		if (curData->isOverDraw)
		{
			continue;
		}

		if (curData->color.x == 0.f &&
			curData->color.y == 0.f &&
			curData->color.z == 0.f)
		{
			continue;
		}

		color->Update(curData->color);



		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(curData->FBX).lock();
		if (curModel != nullptr)
		{

			for (const auto& mesh : curModel->m_Meshes)
			{
				Device->BindMeshBuffer(mesh);

				// Static Mesh Data Update & Bind
				if (!mesh->IsSkinned())
				{
					Device->Context()->IASetInputLayout(m_StaticMeshVS.lock()->InputLayout());
					Device->Context()->VSSetShader(m_StaticMeshVS.lock()->GetVS(), nullptr, 0);
					Device->Context()->PSSetShader(m_RimLightPS.lock()->GetPS(), nullptr, 0);


					TransformData renew;
					XMStoreFloat4x4(&renew.local, XMMatrixTranspose(curData->world));
					XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));
					XMStoreFloat4x4(&renew.localInverse, (curData->world.Invert()));
					XMStoreFloat4x4(&renew.worldInverse, (curData->world.Invert()));
					TransformCB->Update(renew);
					Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), deferredDsv->Get());
				}
				else
				{
					Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), dsv->Get());

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

						Device->Context()->PSSetShaderResources(0, 1, curMaterial->m_NormalSRV.lock()->GetAddress());
					}
				}

				Device->Context()->DrawIndexed(mesh->IBCount(), 0, 0);
			}
		}
	}

	//overdraw pass랑 같은 depth stencil 사용하기 때문에 나중에 클리어
	m_Device.lock()->Context()->ClearDepthStencilView(dsv->Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_Device.lock()->Context()->ClearDepthStencilView(deferredDsv->Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}
void RimLight::OnResize()
{
	std::shared_ptr<ResourceManager> manager = m_ResourceManager.lock();

	m_GbufferSRV = manager->Get<ShaderResourceView>(L"GBuffer").lock();

}