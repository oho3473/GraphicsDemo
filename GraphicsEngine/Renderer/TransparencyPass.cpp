#include "pch.h"
#include "TransparencyPass.h"
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
#include "DepthStencilState.h"

TransparencyPass::TransparencyPass(const std::shared_ptr<Device>& device, std::shared_ptr<ResourceManager> manager)
{
	m_Device = device;
	m_ResourceManager = manager;

	//deferred와 같이 쓰려면 deferred를 연산한 곳에 같이 그려야지
	m_RTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"GBuffer");
	m_DSV = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Deferred");

	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base");

	m_MeshPS = m_ResourceManager.lock()->Get<PixelShader>(L"Mesh");
	m_DebugPS = m_ResourceManager.lock()->Get<PixelShader>(L"BasePS");

	m_BlendState = m_ResourceManager.lock()->Get<BlendState>(L"AlphaBlend");
}

TransparencyPass::~TransparencyPass()
{
	m_RTV.reset();
	m_DSV.reset();
}

void TransparencyPass::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;

	//deferred와 같이 쓰려면 deferred를 연산한 곳에 같이 그려야지
	m_RTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"GBuffer");
	m_DSV = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Deferred");

	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base");

	m_MeshPS = m_ResourceManager.lock()->Get<PixelShader>(L"Mesh");
	m_DebugPS = m_ResourceManager.lock()->Get<PixelShader>(L"BasePS");

	m_BlendState = m_ResourceManager.lock()->Get<BlendState>(L"AlphaBlend");
}

void TransparencyPass::Render()
{
	std::shared_ptr<Device> Device = m_Device.lock();
	//뭔가 rtv가 달라짐 그래서 다시 받아왔음 왜그러지?
	std::shared_ptr<RenderTargetView> rtv = m_ResourceManager.lock()->Get<RenderTargetView>(L"GBuffer").lock();
	std::shared_ptr<DepthStencilView> dsv = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Deferred").lock();

	std::shared_ptr<DepthStencilView> DSV = m_DSV.lock();
	std::shared_ptr<RenderTargetView> RTV = m_RTV.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();
	std::shared_ptr<BlendState> state = m_BlendState.lock();
	std::shared_ptr<DepthStencilState> depth = m_ResourceManager.lock()->Get<DepthStencilState>(L"NoDepthWrites").lock();

	Device->UnBindSRV();
	Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), dsv->Get());
	Device->Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	Device->Context()->OMSetDepthStencilState(nullptr, 1);

	for (const auto& curData: m_RenderList)
	{
		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(curData->FBX).lock();

		bool isTransparency = false;

		if (curModel != nullptr)
		{
			for (auto material : curModel->m_Materials)
			{
				if (material->m_OpacitySRV.lock() != nullptr)
				{
					isTransparency = true;
					break;
				}
			}

			Device->Context()->RSSetState(curModel->RS.lock()->Get());
			if (isTransparency)
			{
				for (auto& mesh : curModel->m_Meshes)
				{
					Device->Context()->IASetVertexBuffers(0, 1, mesh->GetAddressVB(), mesh->VBSize(), mesh->VBOffset());
					Device->Context()->IASetIndexBuffer(mesh->IB(), DXGI_FORMAT_R32_UINT, 0);
					Device->Context()->IASetPrimitiveTopology(mesh->m_primitive);
					Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);

					if (mesh->IsSkinned())
					{
						Device->BindVS(m_SkeletalMeshVS.lock());

						std::shared_ptr<SkinnedMesh> curMesh = std::dynamic_pointer_cast<SkinnedMesh>(mesh);

						// CB Update
						std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();

						TransformData renew;
						XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curData->world));
						renew.local = curMesh->m_node.lock()->m_World;
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
					else
					{
						BindStatic(curData);

						//std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Create<ConstantBuffer<TransformData>>(L"Transform", ConstantBufferType::Default).lock();
					}

					if (!curModel->m_Materials.empty())
					{
						std::shared_ptr<Material> curMaterial = curModel->m_Materials[mesh->m_material];
						std::shared_ptr<ConstantBuffer<MaterialData>> curData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();

						MaterialData curMaterialData = curMaterial->m_Data;
						curData->Update(curMaterialData);

						if (curData->m_struct.useNEOL.z > 0)
						{
							Device->Context()->OMSetBlendState(state->GetState().Get(), nullptr, 0xFFFFFFFF);
							Device->Context()->OMSetDepthStencilState(depth->GetState().Get(), 1);
						}


						Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());
						{
							Device->BindMaterialSRV(curMaterial);
						}
					}

					///Draw
					Device->Context()->DrawIndexed(mesh->IBCount(), 0, 0);
				}
			}
		}
	}

	Device->Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	Device->Context()->OMSetDepthStencilState(nullptr, 1);

}

void TransparencyPass::OnResize()
{
	m_RTV = m_ResourceManager.lock()->Get<RenderTargetView>(L"RTV_Main");
	m_DSV = m_ResourceManager.lock()->Get<DepthStencilView>(L"DSV_Main");

	m_SkeletalMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Skinning");
	m_StaticMeshVS = m_ResourceManager.lock()->Get<VertexShader>(L"Base");

	m_MeshPS = m_ResourceManager.lock()->Get<PixelShader>(L"Mesh");
	m_DebugPS = m_ResourceManager.lock()->Get<PixelShader>(L"BasePS");
}
