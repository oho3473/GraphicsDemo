#pragma once
#include <memory>

#include "RenderPass.h"
#include "ResourceManager.h"
#include "RenderTargetView.h"
#include "LightManager.h"
#include "Slot.h"

class DebugPBRPass :
    public RenderPass
{
public:
	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,const std::shared_ptr<LightManager>& lightManager);


    // RenderPass을(를) 통해 상속됨
    void Render() override;
    void OnResize() override;


private:
	std::weak_ptr<LightManager> m_LightManager;

private:
	std::weak_ptr<DepthStencilView> m_DepthStencilView;

	// Constant Buffers
	std::shared_ptr<ConstantBuffer<TransformData>> m_TransformCB;
	std::shared_ptr<ConstantBuffer<MatrixPallete>> m_SkeletalCB;
	std::weak_ptr<ConstantBuffer<CameraData>> m_CameraCB;

	//srv
	std::weak_ptr<ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr<ShaderResourceView> m_NormalSRV;
	std::weak_ptr<ShaderResourceView> m_MetalicSRV;
	std::weak_ptr<ShaderResourceView> m_RoughnessSRV;

	//ibl
	std::weak_ptr<ShaderResourceView> m_IrrandianceSRV;
	std::weak_ptr<ShaderResourceView> m_RandianceSRV;
	std::weak_ptr<ShaderResourceView> m_LUT;

	//pbr
	std::weak_ptr<RenderTargetView> m_FresnelRTV;
	std::weak_ptr<RenderTargetView> m_DistributeRTV; // 정규 분포
	std::weak_ptr<RenderTargetView> m_GeometryRTV;	 //기하 감쇠
	std::weak_ptr<RenderTargetView> m_SpecularRTV;	 
	std::weak_ptr<RenderTargetView> m_DiffuseRTV;

	std::vector<ID3D11RenderTargetView*> m_PBRs;
};

