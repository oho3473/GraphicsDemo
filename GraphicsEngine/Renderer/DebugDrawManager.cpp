#include "pch.h"
#include "DebugDrawManager.h"
#include "Device.h"
#include "Defines.h"
#include "ResourceManager.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RenderState.h"

using namespace VPMath;

void DebugDrawManager::Initialize(const std::shared_ptr<Device>& device, const std::shared_ptr<ResourceManager>& resourceManager)
{
	m_AlphaBlendBS = resourceManager->Get<BlendState>(L"AlphaBlend").lock();
	m_DefaultDSS = resourceManager->Get<DepthStencilState>(L"DefaultDSS").lock();
	m_CullNoneRS = std::make_shared<RenderState>(device, RasterizerStateType::CullNone);

	m_Batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device->Context());
	m_BatchEffect = std::make_unique<BasicEffect>(device->Get());
	m_BatchEffect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_BatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	HR_CHECK(device->Get()->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		&m_BatchInputLayout));
}

void DebugDrawManager::Execute(const std::shared_ptr<Device>& device, const VPMath::Matrix view, const VPMath::Matrix proj, bool isRender/*=true*/)
{

	if (isRender)
	{

		device->Context()->OMSetBlendState(m_AlphaBlendBS->GetState().Get(), nullptr, 0xFFFFFFFF);
		device->Context()->OMSetDepthStencilState(m_DefaultDSS->GetState().Get(), 0);
		device->Context()->RSSetState(m_CullNoneRS->Get());

		m_BatchEffect->SetView(view);
		m_BatchEffect->SetProjection(proj);
		m_BatchEffect->Apply(device->Context());    //최종 멤버들을 적용한다 먼저 해버리면 이전 프레임의 값이 적용된다

		device->Context()->IASetInputLayout(m_BatchInputLayout.Get());

		m_Batch->Begin();

		while (!m_SphereInfos.empty()) { Draw(m_SphereInfos.front()); m_SphereInfos.pop(); }
		while (!m_BoxInfos.empty()) { Draw(m_BoxInfos.front()); m_BoxInfos.pop(); }
		while (!m_OBBInfos.empty()) { Draw(m_OBBInfos.front()); m_OBBInfos.pop(); }
		while (!m_FrustumInfos.empty()) { Draw(m_FrustumInfos.front()); m_FrustumInfos.pop(); }
		while (!m_GridInfos.empty()) { Draw(m_GridInfos.front()); m_GridInfos.pop(); }
		while (!m_RayInfos.empty()) { Draw(m_RayInfos.front()); m_RayInfos.pop(); }
		while (!m_TriangleInfos.empty()) { Draw(m_TriangleInfos.front()); m_TriangleInfos.pop(); }
		while (!m_QuadInfos.empty()) { Draw(m_QuadInfos.front()); m_QuadInfos.pop(); }
		while (!m_RingInfos.empty()) { DrawRing(m_RingInfos.front()); m_RingInfos.pop(); }

		m_Batch->End();
	}
	else
	{

		while (!m_SphereInfos.empty()) { m_SphereInfos.pop(); }
		while (!m_BoxInfos.empty()) { m_BoxInfos.pop(); }
		while (!m_OBBInfos.empty()) { m_OBBInfos.pop(); }
		while (!m_FrustumInfos.empty()) { m_FrustumInfos.pop(); }
		while (!m_GridInfos.empty()) { m_GridInfos.pop(); }
		while (!m_RayInfos.empty()) { m_RayInfos.pop(); }
		while (!m_TriangleInfos.empty()) { m_TriangleInfos.pop(); }
		while (!m_QuadInfos.empty()) { m_QuadInfos.pop(); }
		while (!m_RingInfos.empty()) { m_RingInfos.pop(); }
	}

	device->Context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	device->Context()->OMSetDepthStencilState(nullptr, 0);
	device->Context()->RSSetState(nullptr);
}

void DebugDrawManager::Draw(const debug::SphereInfo& info)
{
	VPMath::Vector3 origin = info.Sphere.Center;

	const float radius = info.Sphere.Radius;

	VPMath::Vector3 xaxis = VPMath::Vector3::UnitX * radius;
	VPMath::Vector3 yaxis = VPMath::Vector3::UnitY * radius;
	VPMath::Vector3 zaxis = VPMath::Vector3::UnitZ * radius;

	debug::RingInfo ringInfo;
	ringInfo.Origin = origin;
	ringInfo.Color = info.Color;

	ringInfo.MajorAxis = xaxis;
	ringInfo.MinorAxis = zaxis;
	DrawRing(ringInfo);

	ringInfo.MajorAxis = xaxis;
	ringInfo.MinorAxis = yaxis;
	DrawRing(ringInfo);

	ringInfo.MajorAxis = yaxis;
	ringInfo.MinorAxis = zaxis;
	DrawRing(ringInfo);
}

void DebugDrawManager::Draw(const debug::AABBInfo& info)
{
	Matrix matWorld =
		Matrix::CreateScale(info.AABB.Extents) *
		Matrix::CreateTranslation(info.AABB.Center);

	DrawCube(matWorld, info.Color);
}

void DebugDrawManager::Draw(const debug::OBBInfo& info)
{
	Quaternion rotationQuaternion = Quaternion{};

	if (info.OBB.Orientation.x != 0
		|| info.OBB.Orientation.y != 0
		|| info.OBB.Orientation.z != 0
		|| info.OBB.Orientation.w != 1)
	{
		rotationQuaternion = info.OBB.Orientation;

	}
	else
	{
		float xRad = XMConvertToRadians(info.xAxisAngle);
		float yRad = XMConvertToRadians(info.yAxisAngle);
		float zxRad = XMConvertToRadians(info.zAxisAngle);
		rotationQuaternion = Quaternion::CreateFromYawPitchRoll(yRad, xRad, zxRad);
	}

	XMFLOAT4 orientation = info.OBB.Orientation;

	XMStoreFloat4(&orientation, rotationQuaternion);

	Matrix matWorld =
		Matrix::CreateScale(info.OBB.Extents) *
		Matrix::CreateFromQuaternion(orientation) *
		Matrix::CreateTranslation(info.OBB.Center);

	DrawCube(matWorld, info.Color);
}

void DebugDrawManager::Draw(const debug::FrustumInfo& info)
{
	Vector3 corners[BoundingFrustum::CORNER_COUNT];
	info.Frustum.GetCorners(corners);

	VertexPositionColor verts[24] = {};
	verts[0].position = corners[0];
	verts[1].position = corners[1];
	verts[2].position = corners[1];
	verts[3].position = corners[2];
	verts[4].position = corners[2];
	verts[5].position = corners[3];
	verts[6].position = corners[3];
	verts[7].position = corners[0];

	verts[8].position = corners[0];
	verts[9].position = corners[4];
	verts[10].position = corners[1];
	verts[11].position = corners[5];
	verts[12].position = corners[2];
	verts[13].position = corners[6];
	verts[14].position = corners[3];
	verts[15].position = corners[7];

	verts[16].position = corners[4];
	verts[17].position = corners[5];
	verts[18].position = corners[5];
	verts[19].position = corners[6];
	verts[20].position = corners[6];
	verts[21].position = corners[7];
	verts[22].position = corners[7];
	verts[23].position = corners[4];

	for (size_t j = 0; j < std::size(verts); ++j)
	{
		XMStoreFloat4(&verts[j].color, info.Color);
	}

	m_Batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
}

void DebugDrawManager::Draw(const debug::GridInfo& info)
{
	size_t xdivs = std::max<size_t>(1, info.XDivs);
	size_t ydivs = std::max<size_t>(1, info.YDivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float percent = float(i) / float(xdivs);
		percent = (percent * 2.f) - 1.f;
		Vector3 scale = info.XAsix * percent * info.GridSize * 0.5f;
		scale += info.Origin;

		VertexPositionColor v1(scale - info.YAsix * info.GridSize * 0.5f, info.Color);
		VertexPositionColor v2(scale + info.YAsix * info.GridSize * 0.5f, info.Color);
		m_Batch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; ++i)
	{
		float percent = float(i) / float(ydivs);
		percent = (percent * 2.f) - 1.f;
		Vector3 scale = info.YAsix * percent * info.GridSize * 0.5f;
		scale += info.Origin;

		VertexPositionColor v1(scale - info.XAsix * info.GridSize * 0.5f, info.Color);
		VertexPositionColor v2(scale + info.XAsix * info.GridSize * 0.5f, info.Color);
		m_Batch->DrawLine(v1, v2);
	}
}

void DebugDrawManager::Draw(const debug::RayInfo& info)
{
	VertexPositionColor verts[3];
	XMStoreFloat3(&verts[0].position, info.Origin);

	Vector3 normDirection;
	normDirection.Normalize();
	Vector3 rayDirection = (info.Normalize) ? normDirection : info.Direction;

	Vector3 perpVector = normDirection.Cross(Vector3::UnitY);

	if (perpVector.LengthSquared() == 0.0f)
	{
		perpVector = normDirection.Cross(Vector3::UnitZ);
	}
	perpVector.Normalize();

	verts[1].position = rayDirection + info.Origin;
	perpVector *= 0.0625;
	normDirection *= -0.25;
	rayDirection += perpVector;
	rayDirection += normDirection;
	verts[2].position = rayDirection + info.Origin;

	verts[0].color = info.Color;
	verts[1].color = info.Color;
	verts[2].color = info.Color;

	m_Batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
}

void DebugDrawManager::Draw(const debug::TriangleInfo& info)
{
	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, info.PointA);
	XMStoreFloat3(&verts[1].position, info.PointB);
	XMStoreFloat3(&verts[2].position, info.PointC);
	XMStoreFloat3(&verts[3].position, info.PointA);

	XMStoreFloat4(&verts[0].color, info.Color);
	XMStoreFloat4(&verts[1].color, info.Color);
	XMStoreFloat4(&verts[2].color, info.Color);
	XMStoreFloat4(&verts[3].color, info.Color);

	m_Batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
}

void DebugDrawManager::Draw(const debug::QuadInfo& info)
{
	VertexPositionColor verts[5];
	XMStoreFloat3(&verts[0].position, info.PointA);
	XMStoreFloat3(&verts[1].position, info.PointB);
	XMStoreFloat3(&verts[2].position, info.PointC);
	XMStoreFloat3(&verts[3].position, info.PointD);
	XMStoreFloat3(&verts[4].position, info.PointA);

	XMStoreFloat4(&verts[0].color, info.Color);
	XMStoreFloat4(&verts[1].color, info.Color);
	XMStoreFloat4(&verts[2].color, info.Color);
	XMStoreFloat4(&verts[3].color, info.Color);
	XMStoreFloat4(&verts[4].color, info.Color);

	m_Batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
}


void DebugDrawManager::DrawRing(const debug::RingInfo& info)
{
	static const size_t c_ringSegments = 32;

	VertexPositionColor verts[c_ringSegments + 1];

	FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
	// Instead of calling cos/sin for each segment we calculate
	// the sign of the angle delta and then incrementally calculate sin
	// and cosine from then on.
	XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
	XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
	XMVECTOR incrementalSin = XMVectorZero();
	static const XMVECTORF32 s_initialCos =
	{
		1.f, 1.f, 1.f, 1.f
	};
	XMVECTOR incrementalCos = s_initialCos.v;
	for (size_t i = 0; i < c_ringSegments; i++)
	{
		XMVECTOR pos = XMVectorMultiplyAdd(info.MajorAxis, incrementalCos, info.Origin);
		pos = XMVectorMultiplyAdd(info.MinorAxis, incrementalSin, pos);
		XMStoreFloat3(&verts[i].position, pos);
		XMStoreFloat4(&verts[i].color, info.Color);
		// Standard formula to rotate a vector.
		XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
		XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
		incrementalCos = newCos;
		incrementalSin = newSin;
	}
	verts[c_ringSegments] = verts[0];

	m_Batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
}

void DebugDrawManager::DrawCube(const VPMath::Matrix& worldTransform, const VPMath::Color& color)
{
	static const XMVECTORF32 s_verts[8] =
	{
		{ { { -1.f, -1.f, -1.f, 0.f } } },
		{ { {  1.f, -1.f, -1.f, 0.f } } },
		{ { {  1.f, -1.f,  1.f, 0.f } } },
		{ { { -1.f, -1.f,  1.f, 0.f } } },
		{ { { -1.f,  1.f, -1.f, 0.f } } },
		{ { {  1.f,  1.f, -1.f, 0.f } } },
		{ { {  1.f,  1.f,  1.f, 0.f } } },
		{ { { -1.f,  1.f,  1.f, 0.f } } }
	};

	static const WORD s_indices[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	VertexPositionColor verts[8];
	for (size_t i = 0; i < 8; ++i)
	{
		XMVECTOR v = XMVector3Transform(s_verts[i], worldTransform);
		XMStoreFloat3(&verts[i].position, v);
		XMStoreFloat4(&verts[i].color, color);
	}

	m_Batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
}

