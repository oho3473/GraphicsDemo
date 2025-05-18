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
	std::vector<DirectX::XMFLOAT4> m_Kernel;	//�븻 ������ �ݱ� ���ø��� ���� ���� ������
	std::vector<DirectX::XMFLOAT4> m_Noise;	//�븻 ������ �ݱ� ���ø��� ȸ����Ű��� ������

	std::weak_ptr<ShaderResourceView> m_NoiseSRV;

};

