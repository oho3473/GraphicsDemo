#pragma once
#include <wrl/client.h>
#include "Resource.h"

enum class RasterizerStateType
{
	Solid,
	Wire,
	CullNone,
};

class RenderState :public Resource
{
public:
	RenderState(const std::shared_ptr<Device>& device, D3D11_RASTERIZER_DESC desc);

	RenderState(const std::shared_ptr<Device>& device, const RasterizerStateType& type);
	~RenderState();

	void Release() override;
	ID3D11RasterizerState* Get() const;

private:

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RS;
};