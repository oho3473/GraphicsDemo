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
			desc.DepthEnable = TRUE; // ���� �׽�Ʈ Ȱ��ȭ
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS;

			desc.StencilEnable = TRUE; // ���ٽ� Ȱ��ȭ
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0xFF;

			// �ȼ��� �������� �� ���ٽ� �� ����
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // ���ٽ� �� ����
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			desc.BackFace = desc.FrontFace;
		}
		break;
		case DepthStencilStateType::AbleStencil:
		{
			desc.DepthEnable = FALSE; // ���� �׽�Ʈ ��Ȱ��ȭ
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			desc.StencilEnable = TRUE; // ���ٽ� �׽�Ʈ Ȱ��ȭ
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0x00; // ���ٽ� �� ���� �� ��

			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; // ���� 1�� ��츸 ���

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
