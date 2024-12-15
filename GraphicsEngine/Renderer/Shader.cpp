#include "pch.h"

#include "Shader.h"
#include "Device.h"

Shader::Shader(std::wstring filename)
	: Resource(), m_filename(filename), m_VS(nullptr), m_PS(), m_InputLayout(nullptr), m_Desc()
{
}


Shader::Shader(const std::shared_ptr<Device>& device, std::wstring filename)
	: Resource(device), m_filename(filename), m_VS(nullptr), m_PS(nullptr), m_InputLayout(nullptr), m_Desc()
{

}


Shader::Shader(const std::shared_ptr<Device>& device, D3D11_INPUT_ELEMENT_DESC desc, std::wstring filename /*= L"need name"*/)
	:Resource(device), m_filename(filename), m_Desc(desc), m_VS(nullptr), m_PS(nullptr), m_InputLayout(nullptr)
{

}




ID3D11VertexShader* Shader::GetVS() const
{
	return m_VS.Get();
}

ID3D11VertexShader** Shader::GetVSAddress()
{
	return &m_VS;
}

ID3D11PixelShader* Shader::GetPS() const
{
	return m_PS.Get();
}

ID3D11PixelShader** Shader::GetPSAddress()
{
	return &m_PS;
}

ID3D11InputLayout* Shader::InputLayout() const
{
	return m_InputLayout.Get();
}

ID3D11InputLayout** Shader::InputLayoutAddress()
{
	return &m_InputLayout;
}
