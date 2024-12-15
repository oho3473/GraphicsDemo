#pragma once
#include "Resource.h"
#include <string>
#include <wrl/client.h>

/*
 *	Texture 로 쓰이는 리소스
 *	이미지 파일을 불러들여 텍스쳐 리소스로 만들어 가지고 있거나
 *	RTV를 참조하는 텍스쳐를 만들어 가지고 있는다.
 */

class ShaderResourceView : public Resource
{
public:
	enum class Directory
	{
		Texture = 0,
		LightMap = 1
	};

	ShaderResourceView(const std::shared_ptr<Device>& device);

	ShaderResourceView(const std::shared_ptr<Device>& device, std::vector<std::string> filelist, Directory dir = Directory::Texture);
	ShaderResourceView(const std::shared_ptr<Device>& device, const std::wstring& filename, Directory dir = Directory::Texture);
	ShaderResourceView(const std::shared_ptr<Device>& device, const std::shared_ptr<RenderTargetView>& rtv);
	ShaderResourceView(const std::shared_ptr<Device>& device, const std::shared_ptr<Texture2D>& texture2D);

	~ShaderResourceView() = default;

	ID3D11ShaderResourceView* Get() const;
	ID3D11ShaderResourceView** GetAddress();
	void Release() override;

	UINT GetWidth() const { return m_Width; }
	UINT GetHeight() const { return m_Height; }

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	UINT m_Width = 0;
	UINT m_Height = 0;

};

