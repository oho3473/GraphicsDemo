#pragma once
#include <memory>
#include "Buffer.h"
#include "Device.h"
#include "BufferData.h"
#include "Desc.h"
#include "Defines.h"
#include <assert.h>

enum class ConstantBufferType
{
	Default,
	Dynamic
};

template<typename T>
class ConstantBuffer :
	public Buffer
{
public:
	//ConstantBuffer(std::shared_ptr<Device> device);

	// 단일생성자
	ConstantBuffer(const std::shared_ptr<Device>& device, const ConstantBufferType& type);
	~ConstantBuffer() override;

	void Release() override;
	void Update();
	void Update(T cbstruct);

	T m_struct;
};


template<typename T>
void ConstantBuffer<T>::Release()
{
	const ULONG refCount = m_buffer.Reset();
	assert(refCount == 0);
}


template <typename T>
ConstantBuffer<T>::ConstantBuffer(const std::shared_ptr<Device>& device, const ConstantBufferType& type)
	: Buffer(device)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(T);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = 0;

	switch (type)
	{
		case ConstantBufferType::Default:
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
			break;
		}
		case ConstantBufferType::Dynamic:
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			break;
		}
		default:
		{
			return;
		}
	}

	HR_CHECK(device->Get()->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf()));
	HRESULT hr = m_Device.lock()->Get()->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring text = L"Create Failed CB";
		MessageBox(0,text.c_str() ,0,0);
	}
}

//template<typename T>
//ConstantBuffer<T>::ConstantBuffer(std::shared_ptr<Device> device)
//	: Buffer(device), m_struct()
//{
//
//}

template<typename T>
ConstantBuffer<T>::~ConstantBuffer()
{
	ConstantBuffer<T>::Release();
}

template<typename T>
void ConstantBuffer<T>::Update()
{
	m_Device.lock()->Context()->UpdateSubresource(m_buffer.Get(), 0, nullptr, &m_struct, 0, 0);
}

template<typename T>
void ConstantBuffer<T>::Update(T cbstruct)
{
	m_struct = cbstruct;

	m_Device.lock()->Context()->UpdateSubresource(m_buffer.Get(), 0, nullptr, &m_struct, 0, 0);
}