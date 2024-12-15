#pragma once
#include "Buffer.h"

enum class VertexBufferType
{
	Default,
	ParticleInit,
	ParticleDraw
};

class VertexBuffer :
    public Buffer
{
public:
	VertexBuffer() = default;
	VertexBuffer(const std::shared_ptr<Device>& device, UINT count = 0);
	VertexBuffer(const std::shared_ptr<Device>& device, D3D11_BUFFER_DESC desc, D3D11_SUBRESOURCE_DATA data, UINT sizeT);

	template <typename T>
	VertexBuffer(const std::shared_ptr<Device>& device, const std::vector<T>& vertices, const bool& isUseCPUWrite = false);

	~VertexBuffer() override = default;

	UINT* Size();
	UINT* Offset();

	virtual void Update() override;
private:
	UINT m_stride = 0;
	UINT offset = 0;
};

template <typename T>
VertexBuffer::VertexBuffer(const std::shared_ptr<Device>& device, const std::vector<T>& vertices, const bool& isUseCPUWrite)
	: m_stride(sizeof(T))
{
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = isUseCPUWrite ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = (UINT)(sizeof(T) * vertices.size());
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = isUseCPUWrite ? D3D11_CPU_ACCESS_WRITE : 0u;
	desc.MiscFlags = 0u;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = vertices.data();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	device->Get()->CreateBuffer(&desc, &data, &m_buffer);
}