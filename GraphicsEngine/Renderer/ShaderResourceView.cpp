#include "pch.h"
#include "ShaderResourceView.h"
#include "DDSTextureLoader.h"
#include <WICTextureLoader.h>
#include <filesystem>
#include <DirectXTex.h>
#include "Texture2D.h"

#include "RenderTargetView.h"

#include <wrl/client.h>
#include "Defines.h"

#include "Util.h"

ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device) : Resource(device)
{

}

ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, const std::wstring& filename, Directory dir)
	: Resource(device)
{
	std::wstring filePath;

	switch (dir)
	{
		case ShaderResourceView::Directory::Texture:
			filePath = L"..\\Data\\Texture\\" + filename;
			break;
		case ShaderResourceView::Directory::LightMap:
			filePath = L"..\\Data\\LightMap\\" + filename;
			break;
		default:
			break;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

	std::filesystem::path _path(filename);
	std::wstring strExtension = _path.extension();
	std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::towlower);

	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImage;

	if (!std::filesystem::exists(filePath))
	{
		std::string filePathStr = Util::ToMultiByte(filePath);

		// ������ ���� �� �� �ؽ�ó ����
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = 1;  // �ּ� ũ�� 1x1
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // �⺻ 32��Ʈ ����
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.MiscFlags = 0;

		// 1x1 ũ���� ������ �ؽ�ó �����͸� ����
		UINT32 blackPixel = 0x00000000;  // ARGB ������ ������
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &blackPixel;
		initData.SysMemPitch = sizeof(UINT32);  // 4����Ʈ (32��Ʈ) ���� ������

		m_Width = textureDesc.Width;
		m_Height = textureDesc.Height;

		HR_CHECK(device->Get()->CreateTexture2D(&textureDesc, &initData, texture.GetAddressOf()));
		HR_CHECK(device->Get()->CreateShaderResourceView(texture.Get(), nullptr, m_SRV.GetAddressOf()));
		return;
	}

	if (strExtension == L".dds")
	{
		HR_CHECK(LoadFromDDSFile(filePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage));
		HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
	}
	else if (strExtension == L".tga")
	{
		HR_CHECK(DirectX::LoadFromTGAFile(filePath.c_str(), &metadata, scratchImage));
		HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
	}
	else if (strExtension == L".hdr")
	{
		HR_CHECK(DirectX::LoadFromHDRFile(filePath.c_str(), &metadata, scratchImage));
		HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
	}
	else // ��Ÿ..
	{
		HR_CHECK(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage));

		DXGI_FORMAT format = metadata.format;

		bool isHighPrecisionFormat = false;
		switch (format)
		{
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
				isHighPrecisionFormat = true;
				break;
			default:
				isHighPrecisionFormat = false;
				break;
		}

		if (!isHighPrecisionFormat)
		{
			DirectX::ScratchImage mipChain;
			HR_CHECK(DirectX::GenerateMipMaps(
				scratchImage.GetImages(),		// �̹��� ������
				scratchImage.GetImageCount(),	// �̹��� ����
				scratchImage.GetMetadata(),		// ��Ÿ������
				DirectX::TEX_FILTER_DEFAULT,	// ���� �ɼ�
				0,								// ������ �Ӹ� ���� ��
				mipChain));						// ������� ����� ScratchImage

			scratchImage = std::move(mipChain);  // �Ӹ��� ���Ե� �ؽ�ó�� ��ü
		}
		else
		{
			// ���е��� ���� ������ �� �Ӹ� ������ �ǳʶٰų� ������ ó��
		}

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = static_cast<UINT>(metadata.width);
		textureDesc.Height = static_cast<UINT>(metadata.height);
		textureDesc.MipLevels = static_cast<UINT>(scratchImage.GetImageCount()); // �Ӹ� ���� ��
		textureDesc.ArraySize = 1;
		textureDesc.Format = metadata.format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.MiscFlags = 0;

		std::vector<D3D11_SUBRESOURCE_DATA> subresourceData(textureDesc.MipLevels);
		for (UINT i = 0; i < textureDesc.MipLevels; ++i)
		{
			const DirectX::Image* mipImage = scratchImage.GetImage(i, 0, 0);
			subresourceData[i].pSysMem = mipImage->pixels;
			subresourceData[i].SysMemPitch = static_cast<UINT>(mipImage->rowPitch);
			subresourceData[i].SysMemSlicePitch = static_cast<UINT>(mipImage->slicePitch);
		}

		device->Get()->CreateTexture2D(&textureDesc, subresourceData.data(), texture.GetAddressOf());
	}

	m_Width = static_cast<UINT>(metadata.width);
	m_Height = static_cast<UINT>(metadata.height);

	HR_CHECK(device->Get()->CreateShaderResourceView(texture.Get(), nullptr, m_SRV.GetAddressOf()));
}

ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, const std::shared_ptr<RenderTargetView>& rtv) : Resource(device)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rtvTexture;
	rtv->Get()->GetResource(reinterpret_cast<ID3D11Resource**>(rtvTexture.GetAddressOf()));

	D3D11_TEXTURE2D_DESC textureDesc = {};
	rtvTexture->GetDesc(&textureDesc);

	m_Width = textureDesc.Width;
	m_Height = textureDesc.Height;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	HR_CHECK(m_Device.lock()->Get()->CreateShaderResourceView(rtvTexture.Get(), &srvDesc, m_SRV.GetAddressOf()));
}

ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, const std::shared_ptr<Texture2D>& texture2D)
	: Resource(device)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	texture2D->Get()->GetDesc(&textureDesc);

	m_Width = textureDesc.Width;
	m_Height = textureDesc.Height;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	HR_CHECK(m_Device.lock()->Get()->CreateShaderResourceView(texture2D->Get(), &srvDesc, m_SRV.GetAddressOf()));
}



ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, std::vector<std::string> filelist, Directory dir) : Resource(device)
{
	if (!filelist.empty())
	{


		std::wstring filePath;
		//texture array


		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> loadedTextures;
		//������ �ؽ�ó ����
		for (auto& filename : filelist)
		{
			std::wstring wfilename;
			wfilename.assign(filename.begin(), filename.end());

			switch (dir)
			{
				case ShaderResourceView::Directory::Texture:
					filePath = L"..\\Data\\Texture\\" + wfilename;
					break;
				case ShaderResourceView::Directory::LightMap:
					filePath = L"..\\Data\\LightMap\\" + wfilename;
					break;
				default:
					break;
			}



			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

			std::filesystem::path _path(filename);
			std::wstring strExtension = _path.extension();
			std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::towlower);

			DirectX::TexMetadata metadata;
			DirectX::ScratchImage scratchImage;

			if (!std::filesystem::exists(filePath))
			{
				std::string filePathStr = Util::ToMultiByte(filePath);

				// ������ ���� �� �� �ؽ�ó ����
				D3D11_TEXTURE2D_DESC textureDesc = {};
				textureDesc.Width = 1;  // �ּ� ũ�� 1x1
				textureDesc.Height = 1;
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = 1;
				textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // �⺻ 32��Ʈ ����
				textureDesc.SampleDesc.Count = 1;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.MiscFlags = 0;

				// 1x1 ũ���� ������ �ؽ�ó �����͸� ����
				UINT32 blackPixel = 0x00000000;  // ARGB ������ ������
				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = &blackPixel;
				initData.SysMemPitch = sizeof(UINT32);  // 4����Ʈ (32��Ʈ) ���� ������

				m_Width = textureDesc.Width;
				m_Height = textureDesc.Height;

				HR_CHECK(device->Get()->CreateTexture2D(&textureDesc, &initData, texture.GetAddressOf()));
				HR_CHECK(device->Get()->CreateShaderResourceView(texture.Get(), nullptr, m_SRV.GetAddressOf()));
				return;
			}

			if (strExtension == L".dds")
			{
				HR_CHECK(LoadFromDDSFile(filePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage));
				HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
			}
			else if (strExtension == L".tga")
			{
				HR_CHECK(DirectX::LoadFromTGAFile(filePath.c_str(), &metadata, scratchImage));
				HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
			}
			else if (strExtension == L".hdr")
			{
				HR_CHECK(DirectX::LoadFromHDRFile(filePath.c_str(), &metadata, scratchImage));
				HR_CHECK(CreateTexture(device->Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, (ID3D11Resource**)texture.GetAddressOf()));
			}
			else // ��Ÿ..
			{
				HR_CHECK(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage));

				DirectX::ScratchImage mipChain;
				HR_CHECK(DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain));
				scratchImage = std::move(mipChain);  // �Ӹ��� ���Ե� �ؽ�ó�� ��ü

				D3D11_TEXTURE2D_DESC textureDesc = {};
				textureDesc.Width = static_cast<UINT>(metadata.width);
				textureDesc.Height = static_cast<UINT>(metadata.height);
				textureDesc.MipLevels = static_cast<UINT>(scratchImage.GetImageCount()); // �Ӹ� ���� ��
				textureDesc.ArraySize = 1;
				//textureDesc.Format = metadata.format;
				textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;	//�̰� �׸������� �����ϸ� srgb�̰� ���� ó���� �Ǵµ� �׳� urom�̸� ����ó���� ���� �̻�����
				textureDesc.SampleDesc.Count = 1;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.MiscFlags = 0;

				std::vector<D3D11_SUBRESOURCE_DATA> subresourceData(textureDesc.MipLevels);
				for (UINT i = 0; i < textureDesc.MipLevels; ++i)
				{
					const DirectX::Image* mipImage = scratchImage.GetImage(i, 0, 0);
					subresourceData[i].pSysMem = mipImage->pixels;
					subresourceData[i].SysMemPitch = static_cast<UINT>(mipImage->rowPitch);
					subresourceData[i].SysMemSlicePitch = static_cast<UINT>(mipImage->slicePitch);
				}

				device->Get()->CreateTexture2D(&textureDesc, subresourceData.data(), texture.GetAddressOf());

			}

			m_Width = static_cast<UINT>(metadata.width);
			m_Height = static_cast<UINT>(metadata.height);
			loadedTextures.push_back(texture);

		}

		int numTextures = static_cast<int>(filelist.size());
		D3D11_TEXTURE2D_DESC textureArrayDesc = {};
		textureArrayDesc.Width = m_Width; // �ؽ�ó�� �ʺ�
		textureArrayDesc.Height = m_Height; // �ؽ�ó�� ����
		textureArrayDesc.MipLevels = 12; // MipLevels�� 1�� ���� -�о���� �ֵ��� �� 12��
		textureArrayDesc.ArraySize = numTextures; // �迭�� ũ�� (���� �ؽ�ó ��)
		textureArrayDesc.Format = DXGI_FORMAT_BC7_UNORM; // dds ���� DXGI_FORMAT_BC3_UNORM or bc7 //png DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
		textureArrayDesc.SampleDesc.Count = 1; // ��Ƽ ���ø� ��Ȱ��ȭ
		textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureArrayDesc.CPUAccessFlags = 0;
		textureArrayDesc.MiscFlags = 0;

		// Texture2DArray ����
		ID3D11Texture2D* textureArray = nullptr;
		HRESULT hr = m_Device.lock()->Get()->CreateTexture2D(&textureArrayDesc, nullptr, &textureArray);

		for (int i = 0; i < numTextures; ++i) {
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = loadedTextures[i]; // �ε�� �ؽ�ó
			for (UINT mipLevel = 0; mipLevel < textureArrayDesc.MipLevels; ++mipLevel) {
				// �� MIP ������ ���� ũ�� ���
				UINT width = max(1U, m_Width >> mipLevel);  // �ʺ�
				UINT height = max(1U, m_Height >> mipLevel); // ����

				// BC3 ������ ���� ��� ���� ũ�� ���� (4�� ����� ���߱�)
				//BC ������ 4x4 ��� ������ �����͸� ó���ϹǷ�, �ʺ�� ���̰� 4�� ����� �ƴ� ��쿡�� �̸� ������ ����� �մϴ�.
				UINT blockWidth = (width + 3) / 4;  // 4�� ���� ��� ũ��
				UINT blockHeight = (height + 3) / 4; // 4�� ���� ��� ũ��

				// �ҽ� �ڽ� ����
				D3D11_BOX srcBox;
				srcBox.left = 0;
				srcBox.top = 0;
				srcBox.front = 0;
				srcBox.right = blockWidth * 4;		//���� �ʺ� png�� width
				srcBox.bottom = blockHeight * 4;   // ���� ���� png�� height	
				srcBox.back = 1;

				D3D11_TEXTURE2D_DESC temp;
				texture->GetDesc(&temp);

				// CopySubresourceRegion ȣ��
				m_Device.lock()->Context()->CopySubresourceRegion(
					textureArray, // ��� �ؽ�ó �迭
					D3D11CalcSubresource(mipLevel, i, textureArrayDesc.MipLevels), // ��� ���� ���ҽ� �ε���
					0, 0, 0, // ��� ��ǥ
					texture.Get(), // �ҽ� �ؽ�ó
					mipLevel, // �ҽ� �ؽ�ó�� MIP ����
					&srcBox // ������ �ҽ� ����
				);
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureArrayDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // 2D �ؽ�ó �迭�� ����
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 12; // Mip Level ����
		srvDesc.Texture2DArray.FirstArraySlice = 0; // ù ��° �����̽�
		srvDesc.Texture2DArray.ArraySize = numTextures; // �迭 ũ�� (�����̽� ��)

		// SRV ����
		//ID3D11ShaderResourceView* textureArraySRV = nullptr;
		hr = m_Device.lock()->Get()->CreateShaderResourceView(textureArray, &srvDesc, &m_SRV);

		textureArray->Release();
		loadedTextures.clear();
	}
}



ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, const D3D11_TEXTURE2D_DESC& cubeTexDesc): Resource(device)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HR_CHECK(device->Get()->CreateTexture2D(&cubeTexDesc, nullptr, texture.GetAddressOf()));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = cubeTexDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	m_Device.lock()->Get()->CreateShaderResourceView(texture.Get(), &srvDesc, &m_SRV);
}

ShaderResourceView::ShaderResourceView(const std::shared_ptr<Device>& device, const std::shared_ptr<Texture2D>& texture2D, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc) : Resource(device)
{
	m_Device.lock()->Get()->CreateShaderResourceView(texture2D->Get(), &desc, &m_SRV);
}

ID3D11ShaderResourceView* ShaderResourceView::Get() const
{
	return m_SRV.Get();
}

ID3D11ShaderResourceView** ShaderResourceView::GetAddress()
{
	return m_SRV.GetAddressOf();
}

void ShaderResourceView::Release()
{
	ULONG refCount = m_SRV.Reset();
	assert(refCount == 0);
}
