#pragma once
#include "Resource.h"
#include <wrl/client.h>

class Texture2D;

enum class RenderTargetViewType
{
	BackBuffer,	// backbuffer
	OffScreen,	// for deferred rendering
	ObjectMask,
	Outline,
	// ���� HDR �� RTV�� �ʿ��� ���̴�.

};

class RenderTargetView : public Resource
{
public:
	RenderTargetView(const std::shared_ptr<Device>& device);
	RenderTargetView(const std::shared_ptr<Device>& device, const RenderTargetViewType& type, const uint32_t& width, const uint32_t& height);
	~RenderTargetView() override = default ;

	void CreateDownscaledView(const unsigned int& scaleRatio);

	ID3D11RenderTargetView* Get() const;
	ID3D11RenderTargetView** GetAddress();

	void OnResize();
	void Release() override;

private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	RenderTargetViewType m_Type;
};
