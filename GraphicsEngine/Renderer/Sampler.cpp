#include "pch.h"
#include "Sampler.h"
#include "Defines.h"

Sampler::Sampler(const std::shared_ptr<Device>& device, D3D11_SAMPLER_DESC desc) : Resource(device)
{
	m_Device.lock()->Get()->CreateSamplerState(&desc, &m_SamplerState);
}

Sampler::Sampler(const std::shared_ptr<Device>& device, const SamplerStateType& type)
	: Resource(device)
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // �⺻ ���͸��� ���� ���͸�
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // �⺻ �ּ� ���� WRAP
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;  // �⺻ MIP LOD ���̾
	desc.MaxAnisotropy = 1;  // �⺻������ ���漺 ���͸��� ������� ����
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // �⺻ �� �Լ��� ������� ����
	desc.BorderColor[0] = 0.0f;  // �⺻ ��� ������ (0, 0, 0, 0) === ��� ������ ������� ����
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	desc.MinLOD = 0;  // �ּ� MIP ����
	desc.MaxLOD = D3D11_FLOAT32_MAX;  // �ִ� MIP ����

	switch (type)
	{
	case SamplerStateType::Default:
		{
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.MinLOD = -FLT_MAX;
			desc.MaxLOD = FLT_MAX;
		break;
		}
		case SamplerStateType::LinearClamp:
		{
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// �ؽ�ó ��ǥ�� Ŭ����
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			break;
		}
		case SamplerStateType::LinearWrap:
		{
			desc.BorderColor[0] = 1.f;
			desc.BorderColor[1] = 1.f;
			desc.BorderColor[2] = 1.f;
			desc.BorderColor[3] = 1.f;

			break;
		}
		case SamplerStateType::PointClamp:
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// ���� ���͸� ���
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// �ؽ�ó ��ǥ�� Ŭ����
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.BorderColor[0] = 1.f;
			desc.BorderColor[1] = 1.f;
			desc.BorderColor[2] = 1.f;
			desc.BorderColor[3] = 1.f;

			break;
		}
		default:
			break;
	}

	HR_CHECK(device->Get()->CreateSamplerState(&desc, m_SamplerState.GetAddressOf()));
}

Sampler::~Sampler()
{

}

ID3D11SamplerState* Sampler::Get() const
{
	return m_SamplerState.Get();
}

ID3D11SamplerState** Sampler::GetAddress()
{
	return m_SamplerState.GetAddressOf();
}

void Sampler::Release()
{
	m_SamplerState.Reset();
}
