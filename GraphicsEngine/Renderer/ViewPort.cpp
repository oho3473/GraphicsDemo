#include "pch.h"
#include "ViewPort.h"

ViewPort::ViewPort(std::weak_ptr<Device> device, RECT& wndsize)
{
	m_VP = new D3D11_VIEWPORT();
	m_VP->TopLeftX = static_cast<float>(wndsize.left);
	m_VP->TopLeftY = static_cast<float>(wndsize.top);
	m_VP->Width = static_cast<float>(wndsize.right - wndsize.left);
	m_VP->Height = static_cast<float>(wndsize.bottom - wndsize.top);
	m_VP->MinDepth = 0.0f;
	m_VP->MaxDepth = 1.0f;
}

void ViewPort::Release()
{
	delete m_VP;
}

D3D11_VIEWPORT* ViewPort::Get() const
{
	return m_VP;
}
