#pragma once
#include "ConstantBuffer.h"

/*
 *	게임 UI 담당
 *	D3D의 2D 렌더링기법을 이용하여 2D Sprite 를 렌더하고
 *	directxtk 의 SpriteBatch를 사용해 글자를 렌더링하는 패스
 */

class UIPass
{
private:
	struct ImageTransformCB
	{
		VPMath::Matrix World;
		VPMath::Matrix View;
		VPMath::Matrix Projection;
	};

public:
	void Initialize(const std::shared_ptr<class Device>& device,
		const std::shared_ptr<class ResourceManager>& resourceManager,
		const std::shared_ptr<class UIManager>& uiManager);

	void Render();

private:
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::shared_ptr<UIManager> m_UIManager;

	std::shared_ptr<ConstantBuffer<ImageTransformCB>> m_ImageTransformCB;

	std::shared_ptr<PixelShader> m_PixelShader;
	std::shared_ptr<VertexShader> m_VertexShader;
};

