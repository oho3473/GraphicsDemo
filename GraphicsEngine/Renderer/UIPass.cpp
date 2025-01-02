#include "pch.h"
#include "UIPass.h"

#include <memory>

#include "Device.h"
#include "ResourceManager.h"
#include "UIManager.h"

#include "DepthStencilState.h"
#include "BlendState.h"

void UIPass::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,
	const std::shared_ptr<UIManager>& uiManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;
	m_UIManager = uiManager;

	m_VertexShader = std::make_shared<VertexShader>(m_Device, L"Sprite2DVS", "main");
	m_PixelShader = std::make_shared<PixelShader>(m_Device, L"Sprite2DPS", "main");
}

void UIPass::Render()
{
	std::shared_ptr<RenderTargetView> rtv = m_ResourceManager->Get<RenderTargetView>(L"GBuffer").lock();
	std::shared_ptr<DepthStencilView> dsv = m_ResourceManager->Get<DepthStencilView>(L"DSV_Main").lock();
	m_Device->Context()->OMSetRenderTargets(1, rtv->GetAddress(), dsv->Get());

	std::shared_ptr<Sampler> linear = m_ResourceManager->Get<Sampler>(L"LinearWrap").lock();
	m_Device->Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 알파 블랜딩 사용.
	float factor[] = { 0.f, 0.f, 0.f, 0.f };
	m_Device->Context()->OMSetBlendState(m_ResourceManager->Get<BlendState>(L"AlphaBlend").lock()->GetState().Get(), factor, 0xffffffff);

	// 2D 렌더링을 위해 Z 버퍼 끄기
	m_Device->Context()->OMSetDepthStencilState(m_ResourceManager->Get<DepthStencilState>(L"DisableDepth").lock()->GetState().Get(), 0);

	m_Device->Context()->IASetInputLayout(m_VertexShader->InputLayout());
	m_Device->Context()->VSSetShader(m_VertexShader->GetShader(), nullptr, 0);
	m_Device->Context()->PSSetShader(m_PixelShader->GetShader(), nullptr, 0);
	m_Device->Context()->PSSetSamplers(0, 1, linear->GetAddress());

	// Execute Draw Call for all 2D, 3D objects
	m_UIManager->Render();

	// TODO: 2D 렌더링이 완료되었으므로 다시 Z 버퍼 키기
	m_Device->Context()->OMSetDepthStencilState(nullptr, 0);
}
