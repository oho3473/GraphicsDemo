#pragma once
#include "RenderPass.h"

class DebugPass : public RenderPass
{
public:
	DebugPass() = default;

	DebugPass(const std::shared_ptr<Device>& device, std::shared_ptr<ResourceManager> manager, std::shared_ptr<DebugDrawManager> debug);
	~DebugPass();

	void Initialize(const std::shared_ptr<Device>& device,
		const std::shared_ptr<ResourceManager>& resourceManager,
		const std::shared_ptr<DebugDrawManager>& debugDrawManager);

	void Render() override;
	void OnResize() override;
	void ClearQueue();

private:
	std::weak_ptr<DebugDrawManager> m_DebugDrawManager;
	VPMath::Matrix m_View;
	VPMath::Matrix m_Proj;
};
