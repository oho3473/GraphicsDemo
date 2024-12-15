#pragma once
#include <wrl/client.h>

#include "Resource.h"

class Texture2D : public Resource
{
public:
	Texture2D(const std::shared_ptr<Device>& device, D3D11_TEXTURE2D_DESC desc);
	Texture2D(const std::shared_ptr<Device>& device);
	~Texture2D() override;

	void Release() override;

	ID3D11Texture2D* Get() const;
	ID3D11Texture2D** GetAddress();

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_tex;
	D3D11_TEXTURE2D_DESC m_desc;
};
