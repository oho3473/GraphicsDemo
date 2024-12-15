#pragma once
#include "RenderPass.h"

class RenderTargetView;

//��ǻ� gbuffer���� forward�� ���� �׸��ͻ�
class OverDrawPass :
    public RenderPass
{
public:
	OverDrawPass(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager);
	~OverDrawPass();

	virtual void Render() override;
	virtual void OnResize() override;
private:
	//Quad
	std::weak_ptr<PixelShader> m_meshPS;

	std::weak_ptr<RenderTargetView> m_Gbuffer;
	std::weak_ptr<RenderTargetView> m_Normal;

};

