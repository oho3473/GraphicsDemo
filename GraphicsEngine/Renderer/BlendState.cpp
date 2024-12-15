#include "pch.h"
#include "BlendState.h"

#include "Defines.h"
#include <assert.h>

BlendState::BlendState(const std::shared_ptr<Device>& device, BlendStateType type)
{
	CD3D11_BLEND_DESC desc(CD3D11_DEFAULT{});

	switch (type)
	{
		case BlendStateType::Opaque:
			/* Default */
			break;
		case BlendStateType::AlphaBlend:
		{
			desc.RenderTarget[0].BlendEnable = TRUE;	// ºí·»µù È°¼ºÈ­
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;	// ¼Ò½º »ö»ó ºí·»µå ÆÑÅÍ
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;	// ¼Ò½º ¾ËÆÄ ºí·»µå ÆÑÅÍ
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	// ´ë»ó »ö»ó ºí·»µå ÆÑÅÍ
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;		// ´ë»ó ¾ËÆÄ ºí·»µå ÆÑÅÍ
			break;
		}
		case BlendStateType::Additive:
		{
			desc.RenderTarget[0].BlendEnable = TRUE;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			break;
		}
		case BlendStateType::Multiplicative:
		{
			desc.RenderTarget[0].BlendEnable = TRUE;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			break;
		}
	}

	HR_CHECK(device->Get()->CreateBlendState(&desc, m_State.GetAddressOf()));
}

void BlendState::Release()
{
	ULONG refCount = m_State.Reset();
	assert(refCount == 0);
}
