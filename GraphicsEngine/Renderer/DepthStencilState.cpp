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
		case DepthStencilStateType::AbleDepthStencil:
		{
			desc.DepthEnable = TRUE; // 깊이 테스트 활성화
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS;

			desc.StencilEnable = TRUE; // 스텐실 활성화
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0xFF;

			// 픽셀이 렌더링될 때 스텐실 값 설정
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // 스텐실 값 변경
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			desc.BackFace = desc.FrontFace;
		}
		break;
		case DepthStencilStateType::AbleStencil:
		{
			desc.DepthEnable = FALSE; // 깊이 테스트 비활성화
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			desc.StencilEnable = TRUE; // 스텐실 테스트 활성화
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0x00; // 스텐실 값 수정 안 함

			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; // 값이 1인 경우만 통과

			desc.BackFace = desc.FrontFace;
		}
		break;
	}

	(device->Get()->CreateDepthStencilState(&desc, m_State.GetAddressOf()));
}

void DepthStencilState::Release()
{
	const ULONG refCount = m_State.Reset();
	//assert(refCount == 0);
}
