#pragma once
#include<memory>

class Device;
class ResourceManager;

class SSAOPass
{
public:
	SSAOPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager> resourceManager);
	

	void Render();
	void OnResize();

private:
	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;


};

