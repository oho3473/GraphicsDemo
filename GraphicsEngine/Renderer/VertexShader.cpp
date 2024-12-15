#include "pch.h"
#include "VertexShader.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Device.h"

#include <wrl/client.h>

#include "Defines.h"
#include "VertexData.h"

VertexShader::VertexShader(std::shared_ptr<Device>device, std::wstring filename, D3D11_INPUT_ELEMENT_DESC* desc /*= Instancing::Desc*/, int count /*= Instancing::Count*/) : Shader(device, filename)
{
	DWORD shaderFlag = D3DCOMPILE_ENABLE_STRICTNESS;

	std::wstring basePath;
#ifdef _DEBUG
	shaderFlag |= D3DCOMPILE_DEBUG;
	shaderFlag |= D3DCOMPILE_SKIP_OPTIMIZATION;
	basePath = L"..\\..\\..\\VPGraphics\\";
#else
	basePath = L"..\\Data\\HLSL\\";
#endif

	const std::string& shaderModel = "vs_5_0";
	const std::wstring hlslFileBasePath = basePath;
	const std::wstring hlslFileExtension = L".hlsl";
	const std::wstring binaryFileExtension = L".cso";

	std::wstring filePath = hlslFileBasePath + filename + hlslFileExtension;

	D3D_SHADER_MACRO macro[] =
	{
		{"SKINNING",""}, // 매크로 이름과 값을 설정
		{nullptr, nullptr}    // 배열의 끝을 나타내기 위해 nullptr로 끝낸다.
	};

	std::string entryPoint = "main";
	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	// 컴파일 된 것이 있다면 그걸 쓴다.
	if (FAILED(
		D3DCompileFromFile(
			filePath.c_str(),
			NULL,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			shaderModel.c_str(),
			shaderFlag,
			0,
			&blob,
			&blob)))
	{
		filePath = filename + binaryFileExtension;
		HR_CHECK(D3DReadFileToBlob(filePath.c_str(), &blob));
	}

	HRESULT hr;

	hr = (m_Device.lock()->Get()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_VS));
	hr = (m_Device.lock()->Get()->CreateInputLayout(desc, count, blob->GetBufferPointer(), blob->GetBufferSize(), &m_InputLayout));


}

VertexShader::VertexShader(const std::shared_ptr<Device>& device,
	const std::wstring& filename,
	const std::string& entryPoint,
	const D3D_SHADER_MACRO* macro
)
	: Shader(device, filename)
{
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	DWORD shaderFlag = D3DCOMPILE_ENABLE_STRICTNESS;

	std::wstring basePath;
#ifdef _DEBUG
	shaderFlag |= D3DCOMPILE_DEBUG;
	shaderFlag |= D3DCOMPILE_SKIP_OPTIMIZATION;
	basePath = L"..\\..\\..\\VPGraphics\\";
#else
	basePath = L"..\\Data\\HLSL\\";
#endif

	const std::string& shaderModel = "vs_5_0";
	const std::wstring hlslFileBasePath = basePath;
	const std::wstring hlslFileExtension = L".hlsl";
	const std::wstring binaryFileExtension = L".cso";

	std::wstring filePath = hlslFileBasePath + filename + hlslFileExtension;

	// 컴파일 된 것이 있다면 그걸 쓴다.
	if (FAILED(
		D3DCompileFromFile(
			filePath.c_str(),
			macro,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			shaderModel.c_str(),
			shaderFlag,
			0,
			&blob,
			&blob)))
	{
		filePath = filename + binaryFileExtension;
		HR_CHECK(D3DReadFileToBlob(filePath.c_str(), &blob));
	}

	/// Input Layout Reflection
	ID3D11ShaderReflection* pReflector = nullptr;

	// Create Reflector
	D3DReflect(blob->GetBufferPointer(),
		blob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&pReflector);

	// Shader Refection
	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	// Shader Input Layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned inputIndex = 0; inputIndex < shaderDesc.InputParameters; inputIndex++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pReflector->GetInputParameterDesc(inputIndex, &paramDesc);

		// Shader Input Data를 기반으로 생성
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Shader Data 기반으로 DXGI format 설정
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		// 현 InputLayout 데이터 삽입
		inputLayoutDesc.push_back(elementDesc);
	}

	// Shader InputLayout 생성
	HR_CHECK(
		device->Get()->CreateInputLayout(
			&inputLayoutDesc[0],
			(UINT)inputLayoutDesc.size(),
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			&m_InputLayout
		);
	);

	HR_CHECK(device->Get()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_VS));
}


void VertexShader::Release()
{
	m_InputLayout.Reset();
	m_VS.Reset();
}