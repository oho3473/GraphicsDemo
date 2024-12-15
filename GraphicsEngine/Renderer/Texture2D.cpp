#include "pch.h"

#include "Texture2D.h"
#include <d3d11.h>

Texture2D::Texture2D(const std::shared_ptr<Device>& device, D3D11_TEXTURE2D_DESC desc) : Resource(device), m_desc(desc), m_tex(nullptr)
{
	m_Device.lock()->Get()->CreateTexture2D(&m_desc, 0, &m_tex);
}

Texture2D::Texture2D(const std::shared_ptr<Device>& device) : m_tex(),m_desc()
{

}

Texture2D::~Texture2D()
{

}

void Texture2D::Release()
{
	m_tex.Reset();
}

ID3D11Texture2D* Texture2D::Get() const
{
	return m_tex.Get();
}

ID3D11Texture2D** Texture2D::GetAddress()
{
	return m_tex.GetAddressOf();
}
