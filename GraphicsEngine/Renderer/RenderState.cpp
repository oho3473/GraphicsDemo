#include "pch.h"

#include "RenderState.h"

RenderState::RenderState(const std::shared_ptr<Device>& device, D3D11_RASTERIZER_DESC desc) : Resource(device)
{
	m_Device.lock()->Get()->CreateRasterizerState(&desc, &m_RS);
}

// TODO: 타입별로 생성하게 수정수정수정
RenderState::RenderState(const std::shared_ptr<Device>& device, const RasterizerStateType& type)
{
	m_Device = device;

    // D3D11_RASTERIZER_DESC 구조체 초기화
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;               // 폴리곤을 채움
    rasterDesc.CullMode = D3D11_CULL_NONE;                // Cull 없음
    rasterDesc.FrontCounterClockwise = FALSE;             // 시계방향을 앞면으로 간주
    rasterDesc.DepthBias = 0;                             // 깊이 바이어스 없음
    rasterDesc.DepthBiasClamp = 0.0f;                     // 깊이 바이어스 클램프 없음
    rasterDesc.SlopeScaledDepthBias = 0.0f;               // 경사 조정 깊이 바이어스 없음
    rasterDesc.DepthClipEnable = TRUE;                    // 깊이 클립 활성화
    rasterDesc.ScissorEnable = FALSE;                     // Scissor 테스트 비활성화
    rasterDesc.MultisampleEnable = FALSE;                 // 멀티샘플링 비활성화
    rasterDesc.AntialiasedLineEnable = FALSE;             // 안티앨리어싱 선 비활성화

    // Rasterizer 상태 생성
    HRESULT hr = m_Device.lock()->Get()->CreateRasterizerState(&rasterDesc, &m_RS);
    if (FAILED(hr))
    {
        // 오류 처리
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
