#include "pch.h"
#include "DepthStencilState.h"

DepthStencilState::DepthStencilState(const std::shared_ptr<Device>& device, DepthStencilStateType type)
{
	CD3D11_DEPTH_STENCIL_DESC desc(CD3D11_DEFAULT{});

	switch (type)
	{
	case DepthStencilStateType::Default:
		{
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS;
			desc.StencilEnable = false;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace = desc.FrontFace;
		break;
		}
	case DepthStencilStateType::DisableDepth:
		{
			desc.DepthEnable = false;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.DepthFunc = D3D11_COMPARISON_LESS;
			desc.StencilEnable = false;
		break;
		}
	case DepthStencilStateType::NoDepthWrites:
		{
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.DepthFunc = D3D11_COMPARISON_LESS;
			desc.StencilEnable = false;
		break;
		}

	}

	(device->Get()->CreateDepthStencilState(&desc, m_State.GetAddressOf()));
}

void DepthStencilState::Release()
{
	const ULONG refCount = m_State.Reset();
	//assert(refCount == 0);
}
