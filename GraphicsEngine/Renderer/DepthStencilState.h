#pragma once
#include <wrl/client.h>
#include "Resource.h"

enum class DepthStencilStateType
{
	Default,
	DisableDepth,
	NoDepthWrites,
	LessEqual,//etc..

	End
};

class DepthStencilState : public Resource
{
public:
	DepthStencilState(const std::shared_ptr<Device>& device, DepthStencilStateType type);

	void Release() override;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetState() { return m_State; }

private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_State;
};

