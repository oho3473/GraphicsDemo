#pragma once
#include "RenderPass.h"
#include "BlendState.h"

class TransparencyPass :
    public RenderPass
{
public:
	TransparencyPass() = default;

	TransparencyPass(const std::shared_ptr<Device>& device, std::shared_ptr<ResourceManager> manager);
	~TransparencyPass() override;

	void Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager);
	void Render() override;
	void OnResize() override;

private:
	std::weak_ptr<BlendState> m_BlendState;

};

