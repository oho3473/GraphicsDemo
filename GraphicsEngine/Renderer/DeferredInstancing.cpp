#include "pch.h"
#include "DeferredInstancing.h"
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
#include "LightManager.h"
#include "Device.h"

DeferredInstancing::~DeferredInstancing()
{

}

void DeferredInstancing::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager, const std::shared_ptr<LightManager>& lightManager)
{
	m_ResourceManager = resourceManager;
	m_Device = device;

	//entity 최대 갯수만큼 지정해놓으면 새로 사이즈를 바꿀 일이 없다?
	for (int i = 0; i < 10000; i++)
	{
		InstanceData temp;
		m_InstanceDatas.push_back(temp);
	}
	m_InstanceBuffer = m_ResourceManager.lock()->Create<VertexBuffer>(L"InstanceBuffer", m_InstanceDatas, true);
	m_InstancingVS = m_ResourceManager.lock()->Create<VertexShader>(L"InstancingVS", L"InstancingVS");

	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_Device.lock()->Context()->Map(m_InstanceBuffer.lock()->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	InstanceData* dataView = reinterpret_cast<InstanceData*>(mappedData.pData);
	dataView = nullptr;
	m_Device.lock()->Context()->Unmap(m_InstanceBuffer.lock()->Get(), 0);

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

	m_QuadVB = resourceManager->Get<VertexBuffer>(L"Quad_VB");
	m_QuadIB = resourceManager->Get<IndexBuffer>(L"Quad_IB");
	m_QuadVS = resourceManager->Get<VertexShader>(L"Quad");
	m_QuadPS = resourceManager->Get<PixelShader>(L"Quad");

	m_SkinnePS = resourceManager->Get<PixelShader>(L"InstancingSkinnedPS").lock();
	m_Deferred = resourceManager->Get<PixelShader>(L"MeshDeferredLight");
	m_MeshPS = resourceManager->Get<PixelShader>(L"InstancingPS");

	m_AlbedoSRV = resourceManager->Get<ShaderResourceView>(L"Albedo").lock();
	m_NormalSRV = resourceManager->Get<ShaderResourceView>(L"Normal").lock();
	m_PositionSRV = resourceManager->Get<ShaderResourceView>(L"Position").lock();
	m_DepthSRV = resourceManager->Get<ShaderResourceView>(L"Depth").lock();
	m_MetalicSRV = resourceManager->Get<ShaderResourceView>(L"Metalic").lock();
	m_RoughnessSRV = resourceManager->Get<ShaderResourceView>(L"Roughness").lock();
	m_EmissiveSRV = resourceManager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = resourceManager->Get<ShaderResourceView>(L"GBuffer").lock();




}

void DeferredInstancing::Render()
{

	std::vector<RenderData> sortingRender;

	sort(m_RenderList.begin(), m_RenderList.end(),
		[](const std::shared_ptr<RenderData> a, const std::shared_ptr<RenderData> b) {
			return a->ModelID < b->ModelID; }
	);

	bool preSkinned = false;
	int preModelID = -1;
	int curModelID = -1;
	int count = 0;
	//instance buffer
	for (auto& object : m_RenderList)
	{
		bool isTranparency = false;

		if (object->color.x != 0.f ||
			object->color.y != 0.f ||
			object->color.z != 0.f)
		{
			continue;
		}

		if (object->isOverDraw || object->punchEffect)
		{
			continue;
		}

		//유효한 모델을 가지고 있지 않음
		if (object->ModelID < 0)
		{
			continue;
		}

		std::shared_ptr<ModelData> curModel = m_ResourceManager.lock()->Get<ModelData>(object->FBX).lock();
		object->ModelID = curModel->UID;

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
		}

		//첫번째 진행시 초기화
		if (preModelID < 0 && curModelID < 0)
		{
			preModelID = object->ModelID;
			curModelID = object->ModelID;

		}
		else
		{
			curModelID = object->ModelID;
		}


		if (object->isSkinned)
		{
			InstanceSkinnedData temp;
			temp.world = object->world.Transpose();
			temp.worldInverse = object->world.Invert();
			std::wstring id = std::to_wstring(object->EntityID);
			m_InstanceSkinnedDatas.push_back(temp);

		}
		else
		{
			InstanceData temp;
			temp.world = object->world.Transpose();
			temp.worldInverse = object->world.Invert();
			temp.lightmap_offset = object->offset;
			temp.lightmap_tiling = object->tiling;
			temp.lightmap_index.x = static_cast<float>(object->lightmapindex);
			//라이트맵 사용하냐?
			if (object->tiling.x <= 0 || object->tiling.y <= 0)
			{
				temp.lightmap_index.y = 0.f;
			}
			else
			{
				temp.lightmap_index.y = 1.f;
			}


			m_InstanceDatas.push_back(temp);
		}

		if (preModelID == curModelID)
		{
			count++;
		}
		else
		{
			if (preSkinned)
			{
				m_instanceskinnedcount.push(std::pair<int, int>(preModelID, count));
			}
			else
			{
				m_instancecount.push(std::pair<int, int>(preModelID, count));
			}
			count = 1;
		}

		preModelID = curModelID;
		preSkinned = object->isSkinned;
	}

	//끝나고 마지막 메쉬 담기
	if (preSkinned)
	{
		m_instanceskinnedcount.push(std::pair<int, int>(preModelID, count));
	}
	else
	{
		m_instancecount.push(std::pair<int, int>(preModelID, count));
	}


	//instance buffer update
	if (!m_InstanceDatas.empty())
	{

		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_Device.lock()->Context()->Map(m_InstanceBuffer.lock()->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstanceData* dataView = reinterpret_cast<InstanceData*>(mappedData.pData);

		for (int i = 0; i < m_InstanceDatas.size(); i++)
		{
			dataView[i] = m_InstanceDatas[i];
		}
		m_Device.lock()->Context()->Unmap(m_InstanceBuffer.lock()->Get(), 0);
	}


	std::shared_ptr<Device> Device = m_Device.lock();
	std::shared_ptr<Sampler> linear = m_ResourceManager.lock()->Get<Sampler>(L"LinearWrap").lock();

	// Bind Common Resources
	{
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
		RTVs.push_back(m_EmissiveRTV.lock()->Get());
		RTVs.push_back(m_LightMapRTV.lock()->Get());


		Device->Context()->OMSetRenderTargets(GBufferSize, RTVs.data(), m_DepthStencilView.lock()->Get());

		Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);
		Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());


		std::shared_ptr<ConstantBuffer<CameraData>> CameraCB = m_ResourceManager.lock()->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
		std::shared_ptr<ConstantBuffer<TransformData>> TransformCB = m_ResourceManager.lock()->Get<ConstantBuffer<TransformData>>(L"Transform").lock();
		std::shared_ptr<ConstantBuffer<MatrixPallete>>SkeletalCB = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
		std::shared_ptr<ConstantBuffer<MaterialData>> MaterialCB = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
		std::shared_ptr<ConstantBuffer<LightArray>> light = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();

		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());
		Device->Context()->PSSetConstantBuffers(static_cast<UINT>(Slot_B::Camera), 1, CameraCB->GetAddress());



		Device->Context()->VSSetConstantBuffers(1, 1, MaterialCB->GetAddress());
		Device->Context()->PSSetConstantBuffers(1, 1, MaterialCB->GetAddress());


		m_Device.lock()->Context()->IASetInputLayout(m_InstancingVS.lock()->InputLayout());
	}

	//draw static
	DrawStatic();
	Device->Context()->OMSetRenderTargets(0, nullptr, nullptr);

	m_InstanceDatas.clear();
}

void DeferredInstancing::OnResize()
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
	m_MetalicSRV = manager->Get<ShaderResourceView>(L"Metalic_Roughness").lock();
	m_RoughnessSRV = manager->Get<ShaderResourceView>(L"Roughness").lock();
	m_EmissiveSRV = manager->Get<ShaderResourceView>(L"Emissive").lock();
	m_GBufferSRV = manager->Get<ShaderResourceView>(L"GBuffer").lock();
	m_LightMapSRV = manager->Get<ShaderResourceView>(L"LightMap").lock();
}


void DeferredInstancing::DrawStatic()
{
	std::shared_ptr<Device> Device = m_Device.lock();

	if (!m_InstanceDatas.empty())
	{
		Device->Context()->VSSetShader(m_InstancingVS.lock()->GetVS(), nullptr, 0);

		UINT strides[2];
		strides[0] = sizeof(BaseVertex);
		strides[1] = sizeof(InstanceData);

		UINT offsets[2];
		offsets[0] = 0;
		offsets[1] = 0;

		auto lightmap = m_LightManager.lock()->GetLightMaps();
		if (lightmap.lock() != nullptr)
		{
			Device->Context()->PSSetShaderResources(static_cast<UINT>(Slot_T::LightMap), 1, lightmap.lock()->GetAddress());
		}

		int preInstance = 0;
		while (!m_instancecount.empty())
		{
			std::pair<int, int> curInstance = m_instancecount.front();
			std::weak_ptr<ModelData> curModel;
			curModel = m_ResourceManager.lock()->Get(curInstance.first);

			if (curModel.lock() != nullptr)
			{
				for (auto& mesh : curModel.lock()->m_Meshes)
				{
					std::shared_ptr<ConstantBuffer<MaterialData>> curMaterialData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
					std::shared_ptr<Material> curMaterial = curModel.lock()->m_Materials[mesh->m_material];
					if (curMaterial != nullptr)
					{
						MaterialData data = curMaterial->m_Data;
						curMaterialData->Update(data);

						Device->BindMaterialSRV(curMaterial);
					}

					std::vector<ID3D11Buffer*> bufferPointers[2];
					bufferPointers->push_back(mesh->VB());
					bufferPointers->push_back(m_InstanceBuffer.lock()->Get());

					m_Device.lock()->Context()->IASetVertexBuffers(0, 2, bufferPointers->data(), strides, offsets);
					m_Device.lock()->Context()->IASetIndexBuffer(mesh->IB(), DXGI_FORMAT_R32_UINT, 0);
					m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					m_Device.lock()->Context()->DrawIndexedInstanced(mesh->IBCount(), curInstance.second, 0, 0, preInstance);
				}
			}

			preInstance += curInstance.second;
			m_instancecount.pop();
		}
	}
}

void DeferredInstancing::DrawSkinned()
{
	std::shared_ptr<Device> Device = m_Device.lock();

	if (!m_InstanceSkinnedDatas.empty())
	{

		Device->Context()->VSSetShader(m_InstancingSkinnedVS.lock()->GetVS(), nullptr, 0);


		UINT strides[2];
		strides[0] = sizeof(SkinningVertex);
		strides[1] = sizeof(InstanceSkinnedData);

		UINT offsets[2];
		offsets[0] = 0;
		offsets[1] = 0;

		int preInstance = 0;
		while (!m_instanceskinnedcount.empty())
		{
			std::pair<int, int> curInstance = m_instanceskinnedcount.front();
			std::weak_ptr<ModelData> curModel;
			curModel = m_ResourceManager.lock()->Get(curInstance.first);

			if (curModel.lock() != nullptr)
			{
				for (auto& mesh : curModel.lock()->m_Meshes)
				{

					std::shared_ptr<ConstantBuffer<MatrixPallete>> pallete;
					{
						//entity id
						pallete = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
					}
					pallete->Update();
					Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, pallete->GetAddress());

					std::shared_ptr<ConstantBuffer<MaterialData>> curMaterialData = m_ResourceManager.lock()->Get<ConstantBuffer<MaterialData>>(L"MaterialData").lock();
					std::shared_ptr<Material> curMaterial = curModel.lock()->m_Materials[mesh->m_material];
					if (curMaterial != nullptr)
					{
						MaterialData data = curMaterial->m_Data;
						curMaterialData->Update(data);

						Device->BindMaterialSRV(curMaterial);
					}

					std::vector<ID3D11Buffer*> bufferPointers[2];
					bufferPointers->push_back(mesh->VB());
					bufferPointers->push_back(m_InstanceBuffer.lock()->Get());

					m_Device.lock()->Context()->IASetVertexBuffers(0, 2, bufferPointers->data(), strides, offsets);
					m_Device.lock()->Context()->IASetIndexBuffer(mesh->IB(), DXGI_FORMAT_R32_UINT, 0);
					m_Device.lock()->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					m_Device.lock()->Context()->DrawIndexedInstanced(mesh->IBCount(), curInstance.second, 0, 0, preInstance);
				}
			}

			preInstance += curInstance.second;
			m_instanceskinnedcount.pop();
		}
	}
}

void DeferredInstancing::SetRenderQueue(const std::vector<std::shared_ptr<RenderData>>& renderQueue)
{
	m_RenderList = renderQueue;

	/*if (!m_InstanceSkinnedDatas.empty())
	{

		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_Device.lock()->Context()->Map(m_InstanceSkinnedBuffer.lock()->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstanceSkinnedData* dataView = reinterpret_cast<InstanceSkinnedData*>(mappedData.pData);

		for (int i = 0; i < m_InstanceSkinnedDatas.size(); i++)
		{
			dataView[i] = m_InstanceSkinnedDatas[i];
		}
		m_Device.lock()->Context()->Unmap(m_InstanceSkinnedBuffer.lock()->Get(), 0);
	}*/


}