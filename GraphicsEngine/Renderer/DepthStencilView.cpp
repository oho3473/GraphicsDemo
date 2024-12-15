#include "pch.h"	

#include "DepthStencilView.h"

#include "Defines.h"
#include "Texture2D.h"
#include "Desc.h"

DepthStencilView::DepthStencilView(const std::shared_ptr<Device>& device, D3D11_TEXTURE2D_DESC desc) : Resource(device)
{
	D3D11_TEXTURE2D_DESC dsd{};
	dsd.Width = m_Device.lock()->GetWndSize().right - m_Device.lock()->GetWndSize().left;
	dsd.Height = m_Device.lock()->GetWndSize().bottom - m_Device.lock()->GetWndSize().top;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	/*
	if (m_Device->ableMSAA)
	{
		dsd.SampleDesc.Count = 4;
		dsd.SampleDesc.Quality = m_Device->MSAAQuality - 1;
	}
	else
	*/
	{
		dsd.SampleDesc.Count = 1;
		dsd.SampleDesc.Quality = 0;
	}

	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsd.CPUAccessFlags = 0;
	dsd.MiscFlags = 0;

	ID3D11Texture2D* DSBuffer;

	HRESULT hr;
	hr = m_Device.lock()->Get()->CreateTexture2D(&dsd, 0, &DSBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Device CreateTexture Failed", 0, 0);
	}
	else
	{
		hr = m_Device.lock()->Get()->CreateDepthStencilView(DSBuffer, 0, &m_DSV);
	}

	if (FAILED(hr))
	{
		MessageBox(0, L"Device CreateDepthStencilView Failed", 0, 0);
	}

	SAFE_RELEASE_COM(DSBuffer);
}

DepthStencilView::DepthStencilView(const std::shared_ptr<Device>& device, D3D11_DEPTH_STENCIL_VIEW_DESC desc, Texture2D* texture) : Resource(device)
{

	HRESULT hr;
	hr = m_Device.lock()->Get()->CreateDepthStencilView(texture->Get(), &desc, &m_DSV);

	if (FAILED(hr))
	{
		MessageBox(0, L"Device CreateDepthStencilView Failed", 0, 0);
	}
}

DepthStencilView::DepthStencilView(const std::shared_ptr<Device>& device, const DepthStencilViewType& type)
	: Resource(device)
	, m_Type(type)
{
	OnResize();
}

DepthStencilView::DepthStencilView(const std::shared_ptr<Device>& device, D3D11_DEPTH_STENCIL_VIEW_DESC desc) : Resource(device)
{
	/*HRESULT hr;
	hr = m_Device->Get()->CreateDepthStencilView(DSBuffer, 0, &m_DSV);


	if (FAILED(hr))
	{
		MessageBox(0, L"Device CreateDepthStencilView Failed", 0, 0);
	}

	DSBuffer->Release();*/
}

ID3D11DepthStencilView* DepthStencilView::Get() const
{
	return m_DSV.Get();
}

ID3D11DepthStencilView** DepthStencilView::GetAddress()
{
	return &m_DSV;
}

void DepthStencilView::OnResize()
{
	Release();

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice = m_Device.lock()->Get();

	const uint32_t width = m_Device.lock()->GetWndWidth();
	const uint32_t height = m_Device.lock()->GetWndHeight();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	switch (m_Type)
	{
		case DepthStencilViewType::Default:
		{
			textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		break;
		default:
			break;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
	HR_CHECK(d3dDevice->CreateTexture2D(&textureDesc, 0, &buffer));
	HR_CHECK(d3dDevice->CreateDepthStencilView(buffer.Get(), &depthStencilViewDesc, m_DSV.GetAddressOf()));
}

void DepthStencilView::Release()
{
	const ULONG refCount = m_DSV.Reset();
	assert(refCount == 0);
}
