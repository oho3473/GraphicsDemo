#pragma once
#include "RenderPass.h"

class BlendState;



class RimLight : public RenderPass
{
public:
	RimLight(std::shared_ptr<Device> device, std::shared_ptr<ResourceManager> manager);
	~RimLight();

	virtual void Render() override;
	virtual void OnResize() override;
private:
	//Quad
	std::weak_ptr<PixelShader> m_RimLightPS;

	std::weak_ptr<ShaderResourceView> m_GbufferSRV;

};

