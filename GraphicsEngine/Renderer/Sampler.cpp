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
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // 기본 필터링은 선형 필터링
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // 기본 주소 모드는 WRAP
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;  // 기본 MIP LOD 바이어스
	desc.MaxAnisotropy = 1;  // 기본적으로 비등방성 필터링은 사용하지 않음
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // 기본 비교 함수는 사용하지 않음
	desc.BorderColor[0] = 0.0f;  // 기본 경계 색상은 (0, 0, 0, 0) === 경계 색상을 사용하지 않음
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	desc.MinLOD = 0;  // 최소 MIP 레벨
	desc.MaxLOD = D3D11_FLOAT32_MAX;  // 최대 MIP 레벨

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
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// 텍스처 좌표를 클램핑
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
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// 선형 필터링 사용
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// 텍스처 좌표를 클램핑
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
