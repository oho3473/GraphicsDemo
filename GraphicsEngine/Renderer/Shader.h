#pragma once
#include "Resource.h"
#include <string>
#include <d3d11shader.h>
#include <wrl/client.h>

#include "Device.h"

/// <summary>
/// VertexShader와 PixelShader의 부모 클래스
/// Shader를 관리하기 위한 구현 클래스
/// </summary>
class Shader :
	public Resource
{
public:
	Shader(std::wstring filename = L"need name");
	Shader(const std::shared_ptr<Device>& device, std::wstring filename = L"need name");
	Shader(const std::shared_ptr<Device>& device, D3D11_INPUT_ELEMENT_DESC desc, std::wstring filename = L"need name");
	virtual ~Shader() = default;

	virtual void Release() abstract;

	ID3D11VertexShader* GetVS() const;
	ID3D11VertexShader** GetVSAddress();
	ID3D11PixelShader* GetPS() const;
	ID3D11PixelShader** GetPSAddress();
	ID3D11InputLayout* InputLayout() const;
	ID3D11InputLayout** InputLayoutAddress();

protected:
	std::wstring m_filename;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_CS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;

	D3D11_INPUT_ELEMENT_DESC m_Desc = {};//???
};
