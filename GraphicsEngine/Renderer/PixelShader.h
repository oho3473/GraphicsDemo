#pragma once
#include "Shader.h"

#include <string>
#include "Device.h"

class PixelShader : public Shader
{
public:
	//PixelShader(std::wstring filename = L"need name");
	//PixelShader(const std::shared_ptr<Device>& device, std::wstring filename = L"need name");

	PixelShader(const std::shared_ptr<Device>& device,
		const std::wstring& filename,
		const std::string& entryPoint,
		const D3D_SHADER_MACRO* macro = nullptr);

	~PixelShader() override = default;

	ID3D11PixelShader* GetVS() const = delete;
	ID3D11InputLayout* InputLayout() const = delete;
	void Release() override;

	ID3D11PixelShader* GetShader() { return m_PS.Get(); }
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetComPtr() { return m_PS; }

};

