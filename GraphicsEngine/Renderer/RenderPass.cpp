#include "pch.h"
#include "RenderPass.h"
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



RenderPass::~RenderPass()
{
	m_SkeletalMeshVS.reset();
	m_DebugPS.reset();

	m_RTV.reset();
	m_DSV.reset();

	m_Device.reset();
	m_ResourceManager.reset();
}

RenderPass::RenderPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager> resourcemanager) : m_Device(device), m_ResourceManager(resourcemanager)
{

}

void RenderPass::SetRenderQueue(const std::vector<std::shared_ptr<RenderData>>& renderQueue)
{
	m_RenderList = renderQueue;
}

void RenderPass::BindStatic(std::shared_ptr<RenderData> curModel)
{
	std::shared_ptr<Device> Device = m_Device.lock();

	Device->Context()->IASetInputLayout(m_StaticMeshVS.lock()->InputLayout());
	Device->Context()->VSSetShader(m_StaticMeshVS.lock()->GetVS(), nullptr, 0);
	Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);

	std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Create<ConstantBuffer<TransformData>>(L"Transform", ConstantBufferType::Default).lock();

	TransformData renew;
	XMStoreFloat4x4(&renew.local, XMMatrixTranspose(curModel->world));
	XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curModel->world));
	XMStoreFloat4x4(&renew.localInverse, (curModel->world.Invert()));
	XMStoreFloat4x4(&renew.worldInverse, (curModel->world.Invert()));
	position->Update(renew);
}

void RenderPass::BindSkeletal(std::shared_ptr<RenderData> curModel, std::shared_ptr<Mesh> mesh)
{
	std::shared_ptr<Device> Device = m_Device.lock();

	Device->Context()->IASetInputLayout(m_SkeletalMeshVS.lock()->InputLayout());
	Device->Context()->VSSetShader(m_SkeletalMeshVS.lock()->GetVS(), nullptr, 0);
	Device->Context()->PSSetShader(m_MeshPS.lock()->GetPS(), nullptr, 0);

	std::shared_ptr<SkinnedMesh> curMesh = std::dynamic_pointer_cast<SkinnedMesh>(mesh);
	std::shared_ptr<ConstantBuffer<TransformData>> position = m_ResourceManager.lock()->Create<ConstantBuffer<TransformData>>(L"Transform", ConstantBufferType::Default).lock();

	TransformData renew;
	XMStoreFloat4x4(&renew.world, XMMatrixTranspose(curModel->world));
	renew.local = curMesh->m_node.lock()->m_World;

	position->Update(renew);

	if (!curMesh->m_BoneData.empty())
	{
		MatrixPallete matrixPallete = *(curMesh->Matrix_Pallete);

		std::shared_ptr<ConstantBuffer<MatrixPallete>> pallete = m_ResourceManager.lock()->Get<ConstantBuffer<MatrixPallete>>(L"MatrixPallete").lock();
		pallete->Update(matrixPallete);
		Device->Context()->VSSetConstantBuffers(static_cast<UINT>(Slot_B::MatrixPallete), 1, pallete->GetAddress());
	}
}

