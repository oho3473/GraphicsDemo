#pragma once
#include "RenderPass.h"
#include "BufferData.h"

class ViewPort;


class CubeMapPass :
    public RenderPass
{
public:
    CubeMapPass(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourcemanager);
    ~CubeMapPass();


    virtual void Render() override;
    virtual void OnResize() override;

    void SetCamera(const CameraData* cameras);
	void ChangeCubeTex(const std::wstring name);

private:


    std::weak_ptr<ShaderResourceView> m_CubeSRV;
    std::weak_ptr<ViewPort> m_CubeViewPort;
    std::vector<std::weak_ptr<RenderTargetView>> m_CubeRTVs;
    std::weak_ptr<DepthStencilView> m_CubeDSV;

    std::weak_ptr<VertexBuffer> m_CubeVB;
    std::weak_ptr<IndexBuffer> m_CubeIB;

    std::weak_ptr<VertexShader> m_CubeVS;
    std::weak_ptr<PixelShader> m_CubePS;

	CameraData m_CubeMapCameras[6];
};

