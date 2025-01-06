#include "pch.h"
#include <wrl/client.h>
#include "RenderTargetView.h"
#include "Texture2D.h"
#include <assert.h>
#include "Defines.h"

RenderTargetView::RenderTargetView(const std::shared_ptr<Device>& device)
	: Resource(device)
{
}

RenderTargetView::RenderTargetView(const std::shared_ptr<Device>& device, const RenderTargetViewType& type, const uint32_t& width, const uint32_t& height)
	: Resource(device)
	, m_Type(type)
{
	OnResize();
}


void RenderTargetView::CreateDownscaledView(const unsigned int& scaleRatio)
{
	Release();

	const uint32_t width = m_Device.lock()->GetWndWidth();
	const uint32_t height = m_Device.lock()->GetWndHeight();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width / scaleRatio;  // 绵家等 气
	textureDesc.Height = height / scaleRatio;  // 绵家等 臭捞
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> downscaledTexture;
	m_Device.lock()->Get()->CreateTexture2D(&textureDesc, nullptr, downscaledTexture.GetAddressOf());

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

	m_Device.lock()->Get()->CreateRenderTargetView(downscaledTexture.Get(), &rtvDesc, m_RTV.GetAddressOf());
}

ID3D11RenderTargetView* RenderTargetView::Get() const
{
	return m_RTV.Get();
}

ID3D11RenderTargetView** RenderTargetView::GetAddress()
{
	return m_RTV.GetAddressOf();
}

void RenderTargetView::OnResize()
{
	Release();

	const uint32_t width = m_Device.lock()->GetWndWidth();
	const uint32_t height = m_Device.lock()->GetWndHeight();

	switch (m_Type)
	{
		case RenderTargetViewType::BackBuffer:
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
			const Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = m_Device.lock()->SwapChain();

			HR_CHECK(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
			HR_CHECK(m_Device.lock()->Get()->CreateRenderTargetView(backBuffer.Get(), 0, &m_RTV));
		}
		break;
		case RenderTargetViewType::OffScreen:
		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
			HR_CHECK(m_Device.lock()->Get()->CreateTexture2D(&textureDesc, nullptr, &texture));

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = textureDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
			HR_CHECK(m_Device.lock()->Get()->CreateRenderTargetView(texture.Get(), &rtvDesc, &m_RTV));

			break;
		}

		case RenderTargetViewType::CubeMap:
		{
			int CubeMapSize = 256;
			D3D11_TEXTURE2D_DESC cubeTexDesc;
			cubeTexDesc.Width = CubeMapSize;
			cubeTexDesc.Height = CubeMapSize;
			cubeTexDesc.MipLevels = 0;
			cubeTexDesc.ArraySize = 6;
			cubeTexDesc.SampleDesc.Count = 1;
			cubeTexDesc.SampleDesc.Quality = 0;
			cubeTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			cubeTexDesc.Usage = D3D11_USAGE_DEFAULT;
			cubeTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			cubeTexDesc.CPUAccessFlags = 0;
			cubeTexDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = cubeTexDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = 1;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
			HR_CHECK(m_Device.lock()->Get()->CreateTexture2D(&cubeTexDesc, nullptr, &texture));

			for (int i = 0; i < 6; i++)
			{
				rtvDesc.Texture2DArray.FirstArraySlice = i;

				HR_CHECK(m_Device.lock()->Get()->CreateRenderTargetView(texture.Get(), &rtvDesc, &m_RTVArray[i]));
			}


		}
		break;

		case RenderTargetViewType::Outline:
		case RenderTargetViewType::ObjectMask:
		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
			HR_CHECK(m_Device.lock()->Get()->CreateTexture2D(&textureDesc, nullptr, &texture));

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = textureDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
			HR_CHECK(m_Device.lock()->Get()->CreateRenderTargetView(texture.Get(), &rtvDesc, m_RTV.GetAddressOf()));

			break;
		}
		default:
			break;
	}
}

void RenderTargetView::Release()
{
	const ULONG refCount = m_RTV.Reset();
	assert(refCount == 0);


	for (int i = 0; i < 6; i++)
	{
		m_RTVArray[i]->Release();
		m_RTVArray[i] = nullptr;
	}


}
