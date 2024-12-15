#pragma once
#include <wrl/client.h>

#include "Resource.h"

enum class SamplerStateType
{
	Default,
	LinearClamp,
	LinearWrap,
	PointClamp,
};

class Sampler : public Resource
{
public:
	Sampler(const std::shared_ptr<Device>& device, D3D11_SAMPLER_DESC desc);
	Sampler(const std::shared_ptr<Device>& device, const SamplerStateType& type);

	~Sampler() override;

	ID3D11SamplerState* Get() const;
	ID3D11SamplerState** GetAddress();

	void Release() override;

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
};
