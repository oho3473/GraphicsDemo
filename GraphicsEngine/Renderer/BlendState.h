#pragma once
#include <wrl/client.h>

#include "Resource.h"

enum class BlendStateType
{
	Opaque,		/* Default */
	AlphaBlend,
	Additive,
	Multiplicative,
};

class BlendState : public Resource
{
public:
	BlendState(const std::shared_ptr<Device>& device, BlendStateType type = BlendStateType::Opaque);
	void Release() override;

	Microsoft::WRL::ComPtr<ID3D11BlendState> GetState() { return m_State; }

private:
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_State;
};

