#pragma once
#include <wrl/client.h>

#include "Resource.h"

class Texture2D;

enum class DepthStencilViewType
{
	Default,

};

class DepthStencilView : public Resource
{
public:
	DepthStencilView(const std::shared_ptr<Device>& device, D3D11_TEXTURE2D_DESC desc);
	DepthStencilView(const std::shared_ptr<Device>& device, D3D11_DEPTH_STENCIL_VIEW_DESC desc);
	DepthStencilView(const std::shared_ptr<Device>& device, D3D11_DEPTH_STENCIL_VIEW_DESC desc, Texture2D* texture);

	DepthStencilView(const std::shared_ptr<Device>& device, const DepthStencilViewType& type);
	~DepthStencilView() override = default;

	ID3D11DepthStencilView* Get() const;
	ID3D11DepthStencilView** GetAddress();

	void OnResize();
	void Release() override;

private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DSV;

	DepthStencilViewType m_Type;
};

