#include "pch.h"
#include "ImageObject.h"

#include <memory>

#include "Device.h"
#include "ResourceManager.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderResourceView.h"
#include "Util.h"

ImageObject::ImageObject(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager,
	const ui::ImageInfo& info, uint32_t id)
		: m_Device(device), m_ResourceManager(resourceManager), m_ID(id), m_Info(info)
{
	// ȭ�� ũ�� ����
	m_CanvasWidth = m_Device->GetWndSize().right - m_Device->GetWndSize().left;
	m_CanvasHeight = m_Device->GetWndSize().bottom - m_Device->GetWndSize().top;

	// ���� �ؽ�ó �ε�
	if (m_Info.ImagePath.empty())	// �ؽ�ó ��ΰ� ����ִٸ� �⺻ �ؽ�ó ��������.
	{
		m_Texture = m_ResourceManager->Get<ShaderResourceView>(L"DefaultUI").lock();
		m_Info.ImagePath = "DefaultUI.png";
	}
	else
		m_Texture = m_ResourceManager->Create<ShaderResourceView>(Util::ToWideChar(m_Info.ImagePath.c_str()), Util::ToWideChar(m_Info.ImagePath.c_str())).lock();

	// �������� ��Ʈ���� �ȼ��� ũ�⸦ ����
	m_BitmapWidth = m_Texture->GetWidth();
	m_BitmapHeight = m_Texture->GetHeight();

	// ���� ������ ������ ������ �ʱ�ȭ
	m_PrevWidth = -1;
	m_PrevHeight = -1;
	m_PrevScale = -1;

	// ���� �� �ε��� ���� �ʱ�ȭ
	InitializeBuffers();

	// Initialize Constant Buffers
	m_ColorCB = m_ResourceManager->Create<ConstantBuffer<ColorCB>>(L"ImageColorCB", ConstantBufferType::Default).lock();
	m_ImageTransformCB = m_ResourceManager->Create<ConstantBuffer<ImageTransformCB>>(L"ImageTransformCB", ConstantBufferType::Default).lock();
}

void ImageObject::Render()
{
	// ȭ���� �ٸ� ��ġ�� �������ϱ� ���� ���� ���� ���۸� �ٽ� �����Ѵ�.
	UpdateBuffers();

	// �׸��⸦ �غ��ϱ� ���� �׷��� ������ ���ο� ���ؽ��� �ε��� ���۸� ���ε��Ѵ�.
	UINT stride = sizeof(ImageVertex);
	UINT offset = 0;

	if (m_Info.Color.A() == 0)
		return;

	// CB Update
	ColorCB color;
	color.Color = m_Info.Color;
	color.leftPercent = m_Info.LeftPercent;
	color.rightPercent = m_Info.RightPercent;
	color.topPercent = m_Info.TopPercent;
	color.bottomPercent = m_Info.BottomPercent;
	m_ColorCB->Update(color);

	m_Device->Context()->IASetVertexBuffers(0, 1, m_VertexBuffer->GetAddress(), &stride, &offset);
	m_Device->Context()->IASetIndexBuffer(m_IndexBuffer->Get(), DXGI_FORMAT_R32_UINT, 0);
	m_Device->Context()->VSSetConstantBuffers(0, 1, m_ImageTransformCB->GetAddress());
	m_Device->Context()->PSSetShaderResources(0, 1, m_Texture->GetAddress());
	m_Device->Context()->PSSetConstantBuffers(0, 1, m_ColorCB->GetAddress());
	m_Device->Context()->DrawIndexed(m_indexCount, 0, 0);
}

void ImageObject::SetImageInfo(const ui::ImageInfo& info)
{
	m_Info.PosXPercent = info.PosXPercent;
	m_Info.PosYPercent = info.PosYPercent;
	m_Info.Layer = info.Layer;
	m_Info.ImagePath = info.ImagePath;
	m_Info.Color = info.Color;
	m_Info.Scale = info.Scale;
	m_Info.World = info.World;
	m_Info.RenderMode = info.RenderMode;
	m_Info.Billboard = info.Billboard;
	m_Info.LeftPercent = info.LeftPercent;
	m_Info.RightPercent = info.RightPercent;
	m_Info.TopPercent = info.TopPercent;
	m_Info.BottomPercent = info.BottomPercent;

	// ���� �ؽ�ó �ε�
	if (m_Info.ImagePath.empty())	// �ؽ�ó ��ΰ� ����ִٸ� �⺻ �ؽ�ó ��������.
	{
		m_Texture = m_ResourceManager->Get<ShaderResourceView>(L"DefaultUI").lock();
		m_Info.ImagePath = "DefaultUI";
	}
	else
		m_Texture = m_ResourceManager->Create<ShaderResourceView>(Util::ToWideChar(m_Info.ImagePath.c_str()), Util::ToWideChar(m_Info.ImagePath.c_str())).lock();

	m_BitmapWidth = m_Texture->GetWidth();
	m_BitmapHeight = m_Texture->GetHeight();
}

bool ImageObject::InitializeBuffers()
{
	// ���� �迭�� ���� ���� �ε��� �迭�� �ε��� ���� �����Ѵ�.
	// �� ���� �ﰢ���� ������ �ϱ� ������ ���� ���� 6���� �����Ѵ�.
	m_indexCount = m_vertexCount = 6;

	std::vector<ImageVertex> vertices;
	vertices.resize(m_vertexCount);
	if (vertices.empty())
		return false;

	std::fill(vertices.begin(), vertices.end(), ImageVertex{});

	std::vector<unsigned int> indices;
	indices.resize(m_indexCount);
	if (indices.empty())
		return false;

	for (size_t i = 0; i < m_indexCount; ++i)
	{
		indices[i] = static_cast<unsigned int>(i);
	}

	// ���� ���� ������ ����
	m_VertexBuffer = std::make_shared<VertexBuffer>(m_Device, vertices, true);

	// �ε��� ���� ����
	m_IndexBuffer = std::make_shared<IndexBuffer>(m_Device, indices);

	return true;
}

void ImageObject::UpdateBuffers()
{
	float left, right, top, bottom;

	if (m_Info.RenderMode == ui::RenderModeType::WorldSpace)
	{
		m_CanvasWidth = m_BitmapWidth;
		m_CanvasHeight = m_BitmapHeight;

		const std::shared_ptr<ConstantBuffer<CameraData>> cameraCB = m_ResourceManager->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
		DirectX::XMFLOAT3 cameraPos = DirectX::XMFLOAT3(cameraCB->m_struct.viewInverse.Transpose()._41, cameraCB->m_struct.viewInverse.Transpose()._42, cameraCB->m_struct.viewInverse.Transpose()._43);
		const DirectX::XMFLOAT3 imagePos = DirectX::XMFLOAT3(m_Info.World._41, m_Info.World._42, m_Info.World._43);
		const DirectX::XMFLOAT3 imageScale = DirectX::XMFLOAT3(m_Info.World._11, m_Info.World._22, m_Info.World._33);
		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(imageScale.x, imageScale.y, imageScale.z);
		DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(imagePos.x, imagePos.y, imagePos.z);
		DirectX::XMMATRIX worldMatrix = scaleMatrix;

		DirectX::XMMATRIX rotationMatrix;

		if (m_Info.Billboard == ui::BillboardType::Full)
		{
			DirectX::XMMATRIX cameraView = cameraCB->m_struct.view.Transpose();
			// ī�޶� �� ��Ŀ��� ȸ�� �κ� ���� (ȸ�� ��ĸ� ������)
			DirectX::XMMATRIX cameraRotationMatrix = cameraView;
			cameraRotationMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  // ��ġ ��� ����
			// ȸ�� ����� ������� ���� �̹����� ī�޶� �ٶ󺸵��� ��
			rotationMatrix = DirectX::XMMatrixTranspose(cameraRotationMatrix);

			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, rotationMatrix);
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, translateMatrix);

			m_Transform.World = XMMatrixTranspose(worldMatrix);
		}
		else if (m_Info.Billboard == ui::BillboardType::AxisY)
		{
			// ��ũ ź��Ʈ �Լ��� ����Ͽ� ���� ī�޶� ��ġ�� ���ϵ��� ������ �𵨿� �����ؾ��ϴ� ȸ���� ����մϴ�.
			double angle = atan2(imagePos.x - cameraPos.x, imagePos.z - cameraPos.z) * (180 / DirectX::XM_PI);

			// ȸ���� �������� ��ȯ�Ѵ�.
			float rotation = (float)angle * 0.0174532925f;

			rotationMatrix = DirectX::XMMatrixRotationY(rotation);

			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, rotationMatrix);
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, translateMatrix);

			m_Transform.World = XMMatrixTranspose(worldMatrix);
		}
		else    // == ui::BillboardType::None
		{
			m_Transform.World = m_Info.World.Transpose();						
		}

		m_Transform.View = cameraCB->m_struct.view;
		m_Transform.Projection = cameraCB->m_struct.proj;
		m_ImageTransformCB->Update(m_Transform);

		float sizeX = static_cast<float>(m_CanvasWidth) * 0.001f;
		float sizeY = static_cast<float>(m_CanvasHeight) * 0.001f;

		m_ImageRect.left = -sizeX;
		m_ImageRect.top = sizeY;
		m_ImageRect.right = sizeX;
		m_ImageRect.bottom = -sizeY;

		left = -sizeX;
		top = sizeY;
		right = sizeX;
		bottom = -sizeY;
	}
	else    // ui::RenderModeType::ScreenSpaceOverlay
	{
		m_CanvasWidth = m_Device->GetWndWidth();
		m_CanvasHeight = m_Device->GetWndHeight();

		const std::shared_ptr<ConstantBuffer<CameraData>> cameraCB = m_ResourceManager->Get<ConstantBuffer<CameraData>>(L"Camera").lock();
		m_Transform.World = VPMath::Matrix::Identity;
		m_Transform.View = VPMath::Matrix::Identity;
		m_Transform.Projection = cameraCB->m_struct.orthoProj;	// ���翵 ���� ���
		m_ImageTransformCB->Update(m_Transform);

		// �̹����� ������ ������ ���Ͽ� �޶����� �ʾҴٸ� ���۸� ������Ʈ���� �ʴ´�.
		// ���� �����Ӱ� ���Ͽ� ��ġ�� ������ �ʾҴٸ�
		// ���� ���� ���۸� �ٲ��� �ʱ� ������ ������ ����� ���� �� �ִ�.
		if ((m_Info.PosXPercent == m_PrevPosXPercent && m_Info.PosYPercent == m_PrevPosYPercent)
			&& (m_BitmapWidth == m_PrevWidth && m_BitmapHeight == m_PrevHeight)
			&& (m_Info.Scale == m_PrevScale)
			&& (m_CanvasWidth == m_PrevCanvasWidth) && (m_CanvasHeight == m_PrevCanvasHeight))
			return;

		// �ۼ�Ʈ ����� ���� ������ ��ȯ (0.0 ~ 1.0)
		const float relPosX = m_Info.PosXPercent / 100.0f;  // 1%�� 0.01�� ��ȯ
		const float relPosY = m_Info.PosYPercent / 100.0f;  // 1%�� 0.01�� ��ȯ

		// ���ο� �ػ󵵿� ���� �̹��� �߽��� ��ġ�� ����Ѵ�.
		m_ImageCenterPosX = relPosX * static_cast<float>(m_CanvasWidth);  // �̹��� �߽��� X ��ġ
		m_ImageCenterPosY = relPosY * static_cast<float>(m_CanvasHeight); // �̹��� �߽��� Y ��ġ

		// �̹����� �����ϸ��� ũ�� ���
		const float scaledWidth = static_cast<float>(m_BitmapWidth) * m_Info.Scale;
		const float scaledHeight = static_cast<float>(m_BitmapHeight) * m_Info.Scale;

		// �̹��� �߽� �������� ���� ��� ��ǥ�� ���
		m_ImagePosX = m_ImageCenterPosX - (scaledWidth / 2.0f);
		m_ImagePosY = m_ImageCenterPosY - (scaledHeight / 2.0f);

		m_ImageRect.left = static_cast<LONG>(m_ImagePosX);
		m_ImageRect.right = static_cast<LONG>(m_ImagePosX + scaledWidth);
		m_ImageRect.top = static_cast<LONG>(m_ImagePosY);
		m_ImageRect.bottom = static_cast<LONG>(m_ImagePosY + scaledHeight);

		// ��Ʈ���� ��ǥ ���
		left = static_cast<float>(m_CanvasWidth) / 2 * -1 + m_ImagePosX;
		right = left + scaledWidth;
		top = static_cast<float>(m_CanvasHeight) / 2 - m_ImagePosY;
		bottom = top - scaledHeight;
	}

	// ������ �Ǵ� ��ġ�� ũ�⸦ ������Ʈ�Ѵ�.
	m_PrevPosXPercent = m_Info.PosXPercent;
	m_PrevPosYPercent = m_Info.PosYPercent;
	m_PrevScale = m_Info.Scale;
	m_PrevWidth = m_BitmapWidth;
	m_PrevHeight = m_BitmapHeight;
	m_PrevCanvasWidth = m_CanvasWidth;
	m_PrevCanvasHeight = m_CanvasHeight;
	m_PrevWorld = m_Info.World;

	std::vector<ImageVertex> vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	vertices.resize(m_vertexCount);
	if (vertices.empty())
		return;

	// ���� ��ǥ�迡�� ���� ���� (z ��ǥ�� 0���� �����Ѵ�)
	// ���� ���� �迭�� �����͸� �ε��Ѵ�.
	// ù ��° �ﰢ��
	vertices[0].Position = DirectX::XMFLOAT4(left, top, 0.0f, 1.0f);		// Top left.
	vertices[0].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[1].Position = DirectX::XMFLOAT4(right, bottom, 0.0f, 1.0f);	// Bottom right.
	vertices[1].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
	vertices[2].Position = DirectX::XMFLOAT4(left, bottom, 0.0f, 1.0f);		// Bottom left.
	vertices[2].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
	// �� ��° �ﰢ��
	vertices[3].Position = DirectX::XMFLOAT4(left, top, 0.0f, 1.0f);		// Top left.
	vertices[3].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[4].Position = DirectX::XMFLOAT4(right, top, 0.0f, 1.0f);		// Top right.
	vertices[4].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
	vertices[5].Position = DirectX::XMFLOAT4(right, bottom, 0.0f, 1.0f);	// Bottom right.
	vertices[5].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);

	// ���ؽ� ���۸� �� �� �ֵ��� ��ٴ�.
	const HRESULT result = m_Device->Context()->Map(m_VertexBuffer->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return;

	// ���� ������ �����͸� ����Ű�� �����͸� ��´�.
	ImageVertex* verticesPtr = static_cast<ImageVertex*>(mappedResource.pData);

	// �����͸� ���� ���ۿ� �����Ѵ�.
	memcpy(verticesPtr, vertices.data(), sizeof(ImageVertex) * m_vertexCount);

	// ���� ���� ������ ����� �����Ѵ�.
	m_Device->Context()->Unmap(m_VertexBuffer->Get(), 0);
}
