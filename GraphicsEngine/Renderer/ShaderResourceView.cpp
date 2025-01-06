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

		// 파일이 없을 때 빈 텍스처 생성
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = 1;  // 최소 크기 1x1
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 기본 32비트 포맷
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.MiscFlags = 0;

		// 1x1 크기의 검은색 텍스처 데이터를 설정
		UINT32 blackPixel = 0x00000000;  // ARGB 형식의 검은색
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &blackPixel;
		initData.SysMemPitch = sizeof(UINT32);  // 4바이트 (32비트) 색상 데이터

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
	else // 기타..
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
				scratchImage.GetImages(),		// 이미지 데이터
				scratchImage.GetImageCount(),	// 이미지 개수
				scratchImage.GetMetadata(),		// 메타데이터
				DirectX::TEX_FILTER_DEFAULT,	// 필터 옵션
				0,								// 생성할 밉맵 레벨 수
				mipChain));						// 출력으로 사용할 ScratchImage

			scratchImage = std::move(mipChain);  // 밉맵이 포함된 텍스처로 교체
		}
		else
		{
			// 정밀도가 높은 포맷일 때 밉맵 생성을 건너뛰거나 별도로 처리
		}

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = static_cast<UINT>(metadata.width);
		textureDesc.Height = static_cast<UINT>(metadata.height);
		textureDesc.MipLevels = static_cast<UINT>(scratchImage.GetImageCount()); // 밉맵 레벨 수
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
		//각각의 텍스처 생성
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

				// 파일이 없을 때 빈 텍스처 생성
				D3D11_TEXTURE2D_DESC textureDesc = {};
				textureDesc.Width = 1;  // 최소 크기 1x1
				textureDesc.Height = 1;
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = 1;
				textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 기본 32비트 포맷
				textureDesc.SampleDesc.Count = 1;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.MiscFlags = 0;

				// 1x1 크기의 검은색 텍스처 데이터를 설정
				UINT32 blackPixel = 0x00000000;  // ARGB 형식의 검은색
				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = &blackPixel;
				initData.SysMemPitch = sizeof(UINT32);  // 4바이트 (32비트) 색상 데이터

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
			else // 기타..
			{
				HR_CHECK(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage));

				DirectX::ScratchImage mipChain;
				HR_CHECK(DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain));
				scratchImage = std::move(mipChain);  // 밉맵이 포함된 텍스처로 교체

				D3D11_TEXTURE2D_DESC textureDesc = {};
				textureDesc.Width = static_cast<UINT>(metadata.width);
				textureDesc.Height = static_cast<UINT>(metadata.height);
				textureDesc.MipLevels = static_cast<UINT>(scratchImage.GetImageCount()); // 밉맵 레벨 수
				textureDesc.ArraySize = 1;
				//textureDesc.Format = metadata.format;
				textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;	//이게 그림판으로 저장하면 srgb이고 투명 처리가 되는데 그냥 urom이면 투명처리가 뭔가 이상해짐
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
		textureArrayDesc.Width = m_Width; // 텍스처의 너비
		textureArrayDesc.Height = m_Height; // 텍스처의 높이
		textureArrayDesc.MipLevels = 12; // MipLevels을 1로 설정 -읽어오는 애들이 다 12임
		textureArrayDesc.ArraySize = numTextures; // 배열의 크기 (담을 텍스처 수)
		textureArrayDesc.Format = DXGI_FORMAT_BC7_UNORM; // dds 포맷 DXGI_FORMAT_BC3_UNORM or bc7 //png DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
		textureArrayDesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
		textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureArrayDesc.CPUAccessFlags = 0;
		textureArrayDesc.MiscFlags = 0;

		// Texture2DArray 생성
		ID3D11Texture2D* textureArray = nullptr;
		HRESULT hr = m_Device.lock()->Get()->CreateTexture2D(&textureArrayDesc, nullptr, &textureArray);

		for (int i = 0; i < numTextures; ++i) {
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = loadedTextures[i]; // 로드된 텍스처
			for (UINT mipLevel = 0; mipLevel < textureArrayDesc.MipLevels; ++mipLevel) {
				// 각 MIP 레벨에 대한 크기 계산
				UINT width = max(1U, m_Width >> mipLevel);  // 너비
				UINT height = max(1U, m_Height >> mipLevel); // 높이

				// BC3 포맷을 위한 블록 단위 크기 조정 (4의 배수로 맞추기)
				//BC 포맷은 4x4 블록 단위로 데이터를 처리하므로, 너비와 높이가 4의 배수가 아닌 경우에는 이를 적절히 맞춰야 합니다.
				UINT blockWidth = (width + 3) / 4;  // 4로 나눈 블록 크기
				UINT blockHeight = (height + 3) / 4; // 4로 나눈 블록 크기

				// 소스 박스 설정
				D3D11_BOX srcBox;
				srcBox.left = 0;
				srcBox.top = 0;
				srcBox.front = 0;
				srcBox.right = blockWidth * 4;		//계산된 너비 png는 width
				srcBox.bottom = blockHeight * 4;   // 계산된 높이 png는 height	
				srcBox.back = 1;

				D3D11_TEXTURE2D_DESC temp;
				texture->GetDesc(&temp);

				// CopySubresourceRegion 호출
				m_Device.lock()->Context()->CopySubresourceRegion(
					textureArray, // 대상 텍스처 배열
					D3D11CalcSubresource(mipLevel, i, textureArrayDesc.MipLevels), // 대상 서브 리소스 인덱스
					0, 0, 0, // 대상 좌표
					texture.Get(), // 소스 텍스처
					mipLevel, // 소스 텍스처의 MIP 레벨
					&srcBox // 복사할 소스 영역
				);
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureArrayDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // 2D 텍스처 배열로 설정
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 12; // Mip Level 설정
		srvDesc.Texture2DArray.FirstArraySlice = 0; // 첫 번째 슬라이스
		srvDesc.Texture2DArray.ArraySize = numTextures; // 배열 크기 (슬라이스 수)

		// SRV 생성
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
