#pragma once
#include "ConstantBuffer.h"
#include "vpGraphics.h"
// 2D �̹��� ������Ʈ

class ShaderResourceView;
class IndexBuffer;
class VertexBuffer;

class ImageObject
{
private:
	struct ImageVertex
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 TexCoord;
	};

	struct ImageTransformCB
	{
		VPMath::Matrix World;
		VPMath::Matrix View;
		VPMath::Matrix Projection;
	};

	struct ColorCB
	{
		VPMath::Color Color;	// rgba
		float leftPercent;   // ���ʿ��� ������� ���� (0.0f ~ 1.0f)
		float rightPercent;  // �����ʿ��� ������� ���� (0.0f ~ 1.0f)
		float topPercent;    // ���ʿ��� ������� ���� (0.0f ~ 1.0f)
		float bottomPercent; // �Ʒ��ʿ��� ������� ���� (0.0f ~ 1.0f)
	};
	static_assert(sizeof(ColorCB) % 16 == 0, "must be align");

public:
	ImageObject(const std::shared_ptr<Device>& device, 
		const std::shared_ptr<class ResourceManager>& resourceManager,
		const ui::ImageInfo& info, uint32_t id);
	~ImageObject() = default;
	void Render();

	void SetID(uint32_t entityID) { m_ID = entityID; }
	void SetImageInfo(const ui::ImageInfo& info);

	uint32_t GetID() const { return m_ID; }
	uint32_t GetLayer() const { return m_Info.Layer; }
	RECT GetRect() const { return m_ImageRect; }

private:
	bool InitializeBuffers();
	void UpdateBuffers();

private:
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<ResourceManager> m_ResourceManager;

	uint32_t m_ID = 0;
	ui::ImageInfo m_Info;

	// ȭ�� ������� UI �Ŵ����� ������ �մ°� ������.
	uint32_t m_CanvasWidth = 0;		// �������� �� ��Ȯ�� ���� ��ġ�� �ʿ��ϱ� ������ ����.
	uint32_t m_CanvasHeight = 0;
	uint32_t m_BitmapWidth = 0;
	uint32_t m_BitmapHeight = 0;

	float m_PrevPosXPercent = 0.f;	// ���� �����Ӱ� ���Ͽ� ��ġ�� ������ �ʾҴٸ�
	float m_PrevPosYPercent = 0.f;	// ���� ���� ���۸� �ٲ��� �ʱ� ������ ������ ����� ���� �� �ִ�.
	uint32_t m_PrevWidth = -1;
	uint32_t m_PrevHeight = -1;
	float m_PrevScale = -1;
	uint32_t m_PrevCanvasWidth = 0;
	uint32_t m_PrevCanvasHeight = 0;
	VPMath::Matrix m_PrevWorld = VPMath::Matrix::Identity;

	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;

	ImageTransformCB m_Transform;
	std::shared_ptr<ConstantBuffer<ImageTransformCB>> m_ImageTransformCB;
	std::shared_ptr<ConstantBuffer<ColorCB>> m_ColorCB;

	std::shared_ptr<ShaderResourceView> m_Texture;
	
	int m_vertexCount = 0;
	int m_indexCount = 0;

	float m_ImageCenterPosX = 0;
	float m_ImageCenterPosY = 0;
	float m_ImagePosX = 0;
	float m_ImagePosY = 0;

	RECT m_ImageRect{};
};
