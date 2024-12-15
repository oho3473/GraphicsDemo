#include "pch.h"

#include "RenderState.h"

RenderState::RenderState(const std::shared_ptr<Device>& device, D3D11_RASTERIZER_DESC desc) : Resource(device)
{
	m_Device.lock()->Get()->CreateRasterizerState(&desc, &m_RS);
}

// TODO: Ÿ�Ժ��� �����ϰ� ������������
RenderState::RenderState(const std::shared_ptr<Device>& device, const RasterizerStateType& type)
{
	m_Device = device;

    // D3D11_RASTERIZER_DESC ����ü �ʱ�ȭ
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;               // �������� ä��
    rasterDesc.CullMode = D3D11_CULL_NONE;                // Cull ����
    rasterDesc.FrontCounterClockwise = FALSE;             // �ð������ �ո����� ����
    rasterDesc.DepthBias = 0;                             // ���� ���̾ ����
    rasterDesc.DepthBiasClamp = 0.0f;                     // ���� ���̾ Ŭ���� ����
    rasterDesc.SlopeScaledDepthBias = 0.0f;               // ��� ���� ���� ���̾ ����
    rasterDesc.DepthClipEnable = TRUE;                    // ���� Ŭ�� Ȱ��ȭ
    rasterDesc.ScissorEnable = FALSE;                     // Scissor �׽�Ʈ ��Ȱ��ȭ
    rasterDesc.MultisampleEnable = FALSE;                 // ��Ƽ���ø� ��Ȱ��ȭ
    rasterDesc.AntialiasedLineEnable = FALSE;             // ��Ƽ�ٸ���� �� ��Ȱ��ȭ

    // Rasterizer ���� ����
    HRESULT hr = m_Device.lock()->Get()->CreateRasterizerState(&rasterDesc, &m_RS);
    if (FAILED(hr))
    {
        // ���� ó��
        //throw std::runtime_error("Failed to create rasterizer state.");
    }
}

RenderState::~RenderState()
{

}


void RenderState::Release()
{
	m_RS.Reset();
}

ID3D11RasterizerState* RenderState::Get() const
{
	return m_RS.Get();
}
