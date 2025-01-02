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
	// 화면 크기 저장
	m_CanvasWidth = m_Device->GetWndSize().right - m_Device->GetWndSize().left;
	m_CanvasHeight = m_Device->GetWndSize().bottom - m_Device->GetWndSize().top;

	// 모델의 텍스처 로드
	if (m_Info.ImagePath.empty())	// 텍스처 경로가 비어있다면 기본 텍스처 가져오기.
	{
		m_Texture = m_ResourceManager->Get<ShaderResourceView>(L"DefaultUI").lock();
		m_Info.ImagePath = "DefaultUI.png";
	}
	else
		m_Texture = m_ResourceManager->Create<ShaderResourceView>(Util::ToWideChar(m_Info.ImagePath.c_str()), Util::ToWideChar(m_Info.ImagePath.c_str())).lock();

	// 렌더링할 비트맵의 픽셀의 크기를 저장
	m_BitmapWidth = m_Texture->GetWidth();
	m_BitmapHeight = m_Texture->GetHeight();

	// 이전 렌더링 정보를 음수로 초기화
	m_PrevWidth = -1;
	m_PrevHeight = -1;
	m_PrevScale = -1;

	// 정점 및 인덱스 버퍼 초기화
	InitializeBuffers();

	// Initialize Constant Buffers
	m_ColorCB = m_ResourceManager->Create<ConstantBuffer<ColorCB>>(L"ImageColorCB", ConstantBufferType::Default).lock();
	m_ImageTransformCB = m_ResourceManager->Create<ConstantBuffer<ImageTransformCB>>(L"ImageTransformCB", ConstantBufferType::Default).lock();
}

void ImageObject::Render()
{
	// 화면의 다른 위치로 렌더링하기 위해 동적 정점 버퍼를 다시 빌드한다.
	UpdateBuffers();

	// 그리기를 준비하기 위해 그래픽 파이프 라인에 버텍스와 인덱스 버퍼를 바인드한다.
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

	// 모델의 텍스처 로드
	if (m_Info.ImagePath.empty())	// 텍스처 경로가 비어있다면 기본 텍스처 가져오기.
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
	// 정점 배열의 정점 수와 인덱스 배열의 인덱스 수를 지정한다.
	// 두 개의 삼각형을 만들어야 하기 때문의 정점 수를 6으로 설정한다.
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

	// 동적 정점 버퍼의 생성
	m_VertexBuffer = std::make_shared<VertexBuffer>(m_Device, vertices, true);

	// 인덱스 버퍼 생성
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
			// 카메라 뷰 행렬에서 회전 부분 추출 (회전 행렬만 가져옴)
			DirectX::XMMATRIX cameraRotationMatrix = cameraView;
			cameraRotationMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  // 위치 요소 제거
			// 회전 행렬의 역행렬을 구해 이미지가 카메라를 바라보도록 함
			rotationMatrix = DirectX::XMMatrixTranspose(cameraRotationMatrix);

			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, rotationMatrix);
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, translateMatrix);

			m_Transform.World = XMMatrixTranspose(worldMatrix);
		}
		else if (m_Info.Billboard == ui::BillboardType::AxisY)
		{
			// 아크 탄젠트 함수를 사용하여 현재 카메라 위치를 향하도록 빌보드 모델에 적용해야하는 회전을 계산합니다.
			double angle = atan2(imagePos.x - cameraPos.x, imagePos.z - cameraPos.z) * (180 / DirectX::XM_PI);

			// 회전을 라디안으로 변환한다.
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
		m_Transform.Projection = cameraCB->m_struct.orthoProj;	// 정사영 투영 행렬
		m_ImageTransformCB->Update(m_Transform);

		// 이미지의 정보를 이전과 비교하여 달라지지 않았다면 버퍼를 업데이트하지 않는다.
		// 이전 프레임과 비교하여 위치가 변하지 않았다면
		// 동적 정점 버퍼를 바꾸지 않기 때문에 성능의 향상을 꾀할 수 있다.
		if ((m_Info.PosXPercent == m_PrevPosXPercent && m_Info.PosYPercent == m_PrevPosYPercent)
			&& (m_BitmapWidth == m_PrevWidth && m_BitmapHeight == m_PrevHeight)
			&& (m_Info.Scale == m_PrevScale)
			&& (m_CanvasWidth == m_PrevCanvasWidth) && (m_CanvasHeight == m_PrevCanvasHeight))
			return;

		// 퍼센트 기반의 값을 비율로 변환 (0.0 ~ 1.0)
		const float relPosX = m_Info.PosXPercent / 100.0f;  // 1%는 0.01로 변환
		const float relPosY = m_Info.PosYPercent / 100.0f;  // 1%는 0.01로 변환

		// 새로운 해상도에 맞춰 이미지 중심의 위치를 계산한다.
		m_ImageCenterPosX = relPosX * static_cast<float>(m_CanvasWidth);  // 이미지 중심의 X 위치
		m_ImageCenterPosY = relPosY * static_cast<float>(m_CanvasHeight); // 이미지 중심의 Y 위치

		// 이미지의 스케일링된 크기 계산
		const float scaledWidth = static_cast<float>(m_BitmapWidth) * m_Info.Scale;
		const float scaledHeight = static_cast<float>(m_BitmapHeight) * m_Info.Scale;

		// 이미지 중심 기준으로 좌측 상단 좌표를 계산
		m_ImagePosX = m_ImageCenterPosX - (scaledWidth / 2.0f);
		m_ImagePosY = m_ImageCenterPosY - (scaledHeight / 2.0f);

		m_ImageRect.left = static_cast<LONG>(m_ImagePosX);
		m_ImageRect.right = static_cast<LONG>(m_ImagePosX + scaledWidth);
		m_ImageRect.top = static_cast<LONG>(m_ImagePosY);
		m_ImageRect.bottom = static_cast<LONG>(m_ImagePosY + scaledHeight);

		// 비트맵의 좌표 계산
		left = static_cast<float>(m_CanvasWidth) / 2 * -1 + m_ImagePosX;
		right = left + scaledWidth;
		top = static_cast<float>(m_CanvasHeight) / 2 - m_ImagePosY;
		bottom = top - scaledHeight;
	}

	// 렌더링 되는 위치와 크기를 업데이트한다.
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

	// 로컬 좌표계에서 쿼드 생성 (z 좌표는 0으로 고정한다)
	// 동적 정점 배열에 데이터를 로드한다.
	// 첫 번째 삼각형
	vertices[0].Position = DirectX::XMFLOAT4(left, top, 0.0f, 1.0f);		// Top left.
	vertices[0].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[1].Position = DirectX::XMFLOAT4(right, bottom, 0.0f, 1.0f);	// Bottom right.
	vertices[1].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
	vertices[2].Position = DirectX::XMFLOAT4(left, bottom, 0.0f, 1.0f);		// Bottom left.
	vertices[2].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
	// 두 번째 삼각형
	vertices[3].Position = DirectX::XMFLOAT4(left, top, 0.0f, 1.0f);		// Top left.
	vertices[3].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[4].Position = DirectX::XMFLOAT4(right, top, 0.0f, 1.0f);		// Top right.
	vertices[4].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
	vertices[5].Position = DirectX::XMFLOAT4(right, bottom, 0.0f, 1.0f);	// Bottom right.
	vertices[5].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);

	// 버텍스 버퍼를 쓸 수 있도록 잠근다.
	const HRESULT result = m_Device->Context()->Map(m_VertexBuffer->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return;

	// 정점 버퍼의 데이터를 가리키는 포인터를 얻는다.
	ImageVertex* verticesPtr = static_cast<ImageVertex*>(mappedResource.pData);

	// 데이터를 정점 버퍼에 복사한다.
	memcpy(verticesPtr, vertices.data(), sizeof(ImageVertex) * m_vertexCount);

	// 동적 정점 버퍼의 잠금을 해제한다.
	m_Device->Context()->Unmap(m_VertexBuffer->Get(), 0);
}
