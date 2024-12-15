#pragma once
#include "Shader.h"
#include <string>
#include "Desc.h"

class Device;

class VertexShader : public Shader
{
public:
	// 단일 생성자
	VertexShader(const std::shared_ptr<Device>& device,
		const std::wstring& filename,
		const std::string& entryPoint,
		const D3D_SHADER_MACRO* macro = nullptr);

	//인스턴싱용 생성자
	VertexShader(std::shared_ptr<Device>device, std::wstring filename, D3D11_INPUT_ELEMENT_DESC* desc = Instancing::Desc,  int count = Instancing::Count);


	~VertexShader() override = default;

	ID3D11PixelShader* GetPS() = delete;
	ID3D11VertexShader* GetShader() { return m_VS.Get(); }
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetComPtr() { return m_VS; }

	void Release() override;

private:
};