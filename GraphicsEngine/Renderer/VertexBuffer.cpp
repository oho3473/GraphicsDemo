#include "pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const std::shared_ptr<Device>& device, UINT count) : Buffer(device, count)
{

}

VertexBuffer::VertexBuffer(const std::shared_ptr<Device>& device, D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, UINT sizeT) : Buffer(device, desc, data), m_stride(sizeT)
{
	HRESULT hr;
	hr = m_Device.lock()->Get()->CreateBuffer(&m_Desc, &m_Data, &m_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"VertexBuffer Create Failed", 0, 0);
	}
}

UINT* VertexBuffer::Size()
{
	return &m_stride;
}

UINT* VertexBuffer::Offset()
{
	return &offset;
}

void VertexBuffer::Update()
{

}
