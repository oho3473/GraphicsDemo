#pragma once

#include "Device.h"


#include <string>
#include <vector>
#include <memory>


enum class ResourceType
{
	None = 0,
	Shader,
	Buffer,
	RTV,
	DSV,
	SRV,
	RS,
	ModelData,
	PixelShader,
	VertexShader,
	Texture2D,
	Sampler,
	ViewPort,

	End
};

class Shader;
class Buffer;
class RenderTargetView;
class RenderState;
class DepthStencilView;
class ShaderResourceView;
class ModelData;
class PixelShader;
class VertexShader;
class Texture2D;
class Sampler;
class ViewPort;

/// <summary>
/// IResource의 구현 클래스
/// </summary>
class Resource
{
public:
	Resource();
	Resource(const std::shared_ptr<Device>& device);
	virtual ~Resource();

	virtual void Release() abstract;

	template<typename T>
	static ResourceType GetResourceType();

protected:
	std::weak_ptr<Device>m_Device;
private:
};

template<typename T>
ResourceType Resource::GetResourceType()
{
	if (std::is_base_of_v<Buffer, T>)
		return ResourceType::Buffer;
	if (std::is_base_of_v<RenderTargetView, T>)
		return ResourceType::RTV;
	if (std::is_base_of_v<DepthStencilView, T>)
		return ResourceType::DSV;
	if (std::is_base_of_v<ShaderResourceView, T>)
		return ResourceType::SRV;
	if (std::is_base_of_v<RenderState, T>)
		return ResourceType::RS;
	if (std::is_base_of_v<ModelData, T>)
		return ResourceType::ModelData;
	if (std::is_base_of_v<PixelShader, T>)
		return ResourceType::PixelShader;
	if (std::is_base_of_v<VertexShader, T>)
		return ResourceType::VertexShader;
	if (std::is_base_of_v<Texture2D, T>)
		return ResourceType::Texture2D;
	if (std::is_base_of_v<Sampler, T>)
		return ResourceType::Sampler;
	if (std::is_base_of_v<ViewPort, T>)
		return ResourceType::ViewPort;

	return ResourceType::None;
}



