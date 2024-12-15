#pragma once
#include "pch.h"
#include "ShaderResourceView.h"
#include "BufferData.h"

class Device;

//Mesh�� �׸��� ��� �Ǵ� �÷��� �����ϱ� ���� ������ �����ϴ� Ŭ����
//���� ������ �����°� �´°ų�? �ƴѰŰ�����...
class Material
{
public:
	Material(const std::shared_ptr<Device>& device);
	~Material();

	std::wstring AlbeoPath;
	std::wstring NormalPath;
	std::wstring MetalicPath;
	std::wstring RoughnessPath;
	std::wstring AOPath;
	std::wstring EmissivePath;
	std::wstring OpacityPath;

	std::weak_ptr <ShaderResourceView> m_AlbedoSRV;
	std::weak_ptr <ShaderResourceView> m_NormalSRV;
	std::weak_ptr <ShaderResourceView> m_MetalicSRV;
	std::weak_ptr <ShaderResourceView> m_RoughnessSRV;
	std::weak_ptr <ShaderResourceView> m_AOSRV;
	std::weak_ptr <ShaderResourceView> m_EmissiveSRV;
	std::weak_ptr <ShaderResourceView> m_OpacitySRV;

	MaterialData m_Data;
private:
	std::weak_ptr<Device> m_Device;
};