#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>
#include <memory>
#pragma comment(lib, "d3d11.lib")


class RenderTargetView;
class DepthStencilView;
class ShaderResourceView;
class VertexShader;

class Material;
class Mesh;

class Device
{
public:
	Device();
	~Device();

	void Initialize(HWND hWnd);
	void OnResize(bool isFullScreen);

	ID3D11DeviceContext* Context() const;
	ID3D11Device* Get() const;
	IDXGISwapChain* SwapChain() const;
	RECT GetWndSize() const;

	void BeginRender(ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV, const float* color);
	void EndRender();

	void UnBindSRV();
	void BindMaterialSRV(std::shared_ptr<Material> curMaterial);

	void BindMeshBuffer(std::shared_ptr<Mesh> mesh);
	void BindVS(std::shared_ptr<VertexShader> vs);

	uint32_t GetWndWidth() const { return m_wndSize.right - m_wndSize.left;	}
	uint32_t GetWndHeight() const { return m_wndSize.bottom - m_wndSize.top; }

	bool ableMSAA;
	UINT MSAAQuality;

private:
	bool CreateSwapChain();

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;

	//Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	//Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	D3D_FEATURE_LEVEL m_FeatureLevel;

	HWND m_hWnd = nullptr;
	RECT m_wndSize;
	bool m_isFullScreen = false;
};

