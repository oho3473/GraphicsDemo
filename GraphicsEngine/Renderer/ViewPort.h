#pragma once
#include "Resource.h"
#include <windows.h>

class ViewPort :
    public Resource
{
public:
	ViewPort(std::weak_ptr<Device> device, RECT& wndsize);
	virtual void Release() override;

	D3D11_VIEWPORT* Get() const;

private:
	D3D11_VIEWPORT* m_VP;
};

