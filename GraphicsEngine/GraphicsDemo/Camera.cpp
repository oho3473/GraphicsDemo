#include "Camera.h"
#include <windowsx.h>


Camera::Camera() : m_pos(0, 0, 0), m_forward(0, 0, 1), m_up(0, 1, 0), m_right(1, 0, 0), m_nearZ(0), m_farZ(0), m_FOV(0), m_nearWindowHeight(0), m_farWindowHeight(0), m_ratio(1),
m_worldMT(DirectX::XMMatrixIdentity()), m_viewMT(DirectX::XMMatrixIdentity()), m_projMT(DirectX::XMMatrixIdentity()), m_worldviewprojMT(DirectX::XMMatrixIdentity()),
m_world(), m_view(), m_proj(), m_worldviewproj(), m_input(nullptr), m_moveSpeed(1)
{

}

Camera::~Camera()
{

}

void Camera::Initialize(double ratio)
{
	m_ratio = static_cast<float>(ratio);
	m_worldMT = DirectX::XMMatrixIdentity();
	m_viewMT = DirectX::XMMatrixIdentity();
	m_projMT = DirectX::XMMatrixIdentity();
	m_worldviewprojMT = DirectX::XMMatrixIdentity();

	XMStoreFloat4x4(&m_world, m_worldMT);
	XMStoreFloat4x4(&m_view, m_viewMT);
	XMStoreFloat4x4(&m_proj, m_projMT);
	XMStoreFloat4x4(&m_worldviewproj, m_projMT);

	m_input = InputManager::GetInstance();

	m_FOV = 0.25f * 3.14f;
	m_nearZ = 0.1f;
	m_farZ = 1000;

	m_nearWindowHeight = 2 * m_nearZ * tanf(0.5f * m_FOV);
	m_farWindowHeight = 2 * m_farZ * tanf(0.5f * m_FOV);

	m_projMT = DirectX::XMMatrixPerspectiveFovLH(m_FOV, m_ratio, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, m_projMT);
}

void Camera::Update(double dt)
{
	float dtf = static_cast<float>(dt);


	if (m_input->IsKeyDown(VK_SHIFT) || m_input->IsKeyPress(VK_SHIFT))
	{
		m_moveSpeed = 50;
	}
	else
	{
		m_moveSpeed = 10;
	}


	if (m_input->IsKeyDown('S') || m_input->IsKeyPress('S'))
	{
		Walk(m_moveSpeed * -dtf);
	}

	if (m_input->IsKeyDown('W') || m_input->IsKeyPress('W'))
	{
		Walk(m_moveSpeed * dtf);
	}

	if (m_input->IsKeyDown('A') || m_input->IsKeyPress('A'))
	{
		Strafe(m_moveSpeed * -dtf);
	}

	if (m_input->IsKeyDown('D') || m_input->IsKeyPress('D'))
	{
		Strafe(m_moveSpeed * dtf);
	}


	if (m_input->IsKeyDown('Q') || m_input->IsKeyPress('Q'))
	{
		UpDown(m_moveSpeed * -dtf);
	}

	if (m_input->IsKeyDown('E') || m_input->IsKeyPress('E'))
	{
		UpDown(m_moveSpeed * dtf);
	}

	/*
	마우스 회전
	*/

	if (m_input->IsKeyPress(VK_RBUTTON))
	{

		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(m_input->GetDeltMouseX()));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(m_input->GetDeltMouseY()));
		RotateY(dx);
		Pitch(dy);
	}

	if (m_input->IsKeyDown('Z') || m_input->IsKeyPress('Z'))
	{
		m_FOV += dtf;
		m_projMT = DirectX::XMMatrixPerspectiveFovLH(m_FOV, m_ratio, m_nearZ, m_farZ);
		XMStoreFloat4x4(&m_proj, m_projMT);
	}

	if (m_input->IsKeyDown('X') || m_input->IsKeyPress('X'))
	{
		m_FOV -= dtf;
		m_projMT = DirectX::XMMatrixPerspectiveFovLH(m_FOV, m_ratio, m_nearZ, m_farZ);
		XMStoreFloat4x4(&m_proj, m_projMT);
	}

	UpdateView();

	m_worldviewprojMT = m_worldMT * m_viewMT * m_projMT;
}

DirectX::XMFLOAT3 Camera::GetPosition() const
{
	return m_pos;
}

DirectX::XMFLOAT4X4 Camera::World() const
{
	return m_world;
}

DirectX::XMFLOAT4X4 Camera::View() const
{
	return m_view;
}


DirectX::XMFLOAT4X4 Camera::Proj() const
{
	return m_proj;
}

DirectX::XMFLOAT4X4 Camera::OrthoProj() const
{
	return m_orthoProj;
}

void Camera::Walk(float d)
{
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
	DirectX::XMVECTOR f = DirectX::XMLoadFloat3(&m_forward);
	DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_pos);
	XMStoreFloat3(&m_pos, DirectX::XMVectorMultiplyAdd(s, f, p));
}

void Camera::UpDown(float d)
{
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
	DirectX::XMVECTOR u = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_pos);
	XMStoreFloat3(&m_pos, DirectX::XMVectorMultiplyAdd(s, u, p));
}


void Camera::Strafe(float d)
{
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
	DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_pos);
	XMStoreFloat3(&m_pos, DirectX::XMVectorMultiplyAdd(s, r, p));
}

void Camera::Pitch(float angle)
{
	DirectX::XMMATRIX r = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);

	XMStoreFloat3(&m_right, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_right), r));
	XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_up), r));
	XMStoreFloat3(&m_forward, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_forward), r));
}


void Camera::Roll(float angle)
{
	DirectX::XMMATRIX r = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&m_forward), angle);

	XMStoreFloat3(&m_right, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_right), r));
	XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_up), r));
	XMStoreFloat3(&m_forward, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_forward), r));
}


void Camera::Yaw(float angle)
{
	DirectX::XMMATRIX r = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&m_up), angle);

	XMStoreFloat3(&m_right, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_right), r));
	XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_up), r));
	XMStoreFloat3(&m_forward, DirectX::XMVector3TransformNormal(XMLoadFloat3(&m_forward), r));
}

void Camera::RotateX(float angle)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationX(angle);

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
	XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), R));
}

void Camera::RotateY(float angle)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
	XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), R));
}

void Camera::RotateZ(float angle)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(angle);

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
	XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), R));
}

DirectX::XMFLOAT4X4 Camera::WorldViewProj()
{
	XMStoreFloat4x4(&m_worldviewproj, m_worldviewprojMT);
	return m_worldviewproj;
}

void Camera::SetPos(DirectX::XMFLOAT3 pos)
{
	m_pos = pos;
}

void Camera::OnResize(double ratio)
{
	m_ratio = static_cast<float>(ratio);
	XMStoreFloat4x4(&m_worldviewproj, m_projMT);

	m_projMT = DirectX::XMMatrixPerspectiveFovLH(m_FOV, m_ratio, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, m_projMT);
}

void Camera::UpdateView()
{
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&m_forward);
	DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_pos);

	F = DirectX::XMVector3Normalize(F);
	U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(F, R));
	R = DirectX::XMVector3Cross(U, F);

	float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, R));
	float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, U));
	float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, F));

	DirectX::XMStoreFloat3(&m_right, R);
	DirectX::XMStoreFloat3(&m_forward, F);
	DirectX::XMStoreFloat3(&m_up, U);

	m_view._11 = m_right.x;
	m_view._21 = m_right.y;
	m_view._31 = m_right.z;
	m_view._41 = x;

	m_view._12 = m_up.x;
	m_view._22 = m_up.y;
	m_view._32 = m_up.z;
	m_view._42 = y;

	m_view._13 = m_forward.x;
	m_view._23 = m_forward.y;
	m_view._33 = m_forward.z;
	m_view._43 = z;


	m_view._14 = 0.0f;
	m_view._24 = 0.0f;
	m_view._34 = 0.0f;
	m_view._44 = 1.0f;

	m_viewMT = XMLoadFloat4x4(&m_view);
}

