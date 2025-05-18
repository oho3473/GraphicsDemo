#pragma once
#include <memory>
#include <vector>
#include "SimpleMath.h"

class Device;
class ResourceManager;
class ShaderResourceView;

class SSAOPass
{
public:
	SSAOPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager> resourceManager);
	

	void Render();
	void OnResize();
	void Initialize();

private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;

	//ssao kernel
private:
	int m_KernelSize;
	int m_NoiseSize;
	std::vector<DirectX::XMFLOAT4> m_Kernel;	//노말 기준의 반구 샘플링을 위한 분포 난수값
	std::vector<DirectX::XMFLOAT4> m_Noise;	//노말 기준의 반구 샘플링을 회전시키기는 난수값

	std::weak_ptr<ShaderResourceView> m_NoiseSRV;

};

