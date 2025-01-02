#pragma once
#include "ConstantBuffer.h"

/*
 *	���� UI ���
 *	D3D�� 2D ����������� �̿��Ͽ� 2D Sprite �� �����ϰ�
 *	directxtk �� SpriteBatch�� ����� ���ڸ� �������ϴ� �н�
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

