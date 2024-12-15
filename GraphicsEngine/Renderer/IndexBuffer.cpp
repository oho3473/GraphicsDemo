#include "pch.h"

#include "IndexBuffer.h"
#include <d3d11.h>
#include <assert.h>

IndexBuffer::IndexBuffer(const std::shared_ptr<Device>& device, UINT count) : Buffer(device, count)
{

}

IndexBuffer::IndexBuffer()
{

}

IndexBuffer::IndexBuffer(const std::shared_ptr<Device>& device, D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, int drawindex) : Buffer(device, desc, data, drawindex)
{
	HRESULT hr;
	hr = m_Device.lock()->Get()->CreateBuffer(&m_Desc, &m_Data, &m_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"IndexBuffer Create Failed", 0, 0);
	}
}

IndexBuffer::IndexBuffer(const std::shared_ptr<Device>& device, const std::vector<unsigned>& indices)
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0u;
	desc.ByteWidth = (UINT)(sizeof(unsigned int) * indices.size());

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = indices.data();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	device->Get()->CreateBuffer(&desc, &data, &m_buffer);
}

void IndexBuffer::Update()
{

}

void IndexBuffer::Release()
{
	const ULONG refCount = m_buffer.Reset();
	assert(refCount == 0);
}
