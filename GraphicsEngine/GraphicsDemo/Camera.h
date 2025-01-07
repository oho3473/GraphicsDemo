#pragma once
#pragma once

#include "SimpleMath.h"
#include "InputManager.h"

class Camera
{
public:
	Camera();
	Camera(DirectX::XMFLOAT3 forward, DirectX::XMFLOAT3 up, DirectX::XMFLOAT3 right);
	~Camera();

	void Initialize(double ratio);
	void InitializeCube(double ratio);
	void Update(double dt);
	void CubeMapUpdate(double dt, DirectX::XMMATRIX view);
	DirectX::XMFLOAT3 GetPosition() const;

	DirectX::XMFLOAT4X4 World() const;
	DirectX::XMFLOAT4X4 View() const;
	DirectX::XMFLOAT4X4 Proj() const;
	DirectX::XMFLOAT4X4 OrthoProj() const;

	void Strafe(float d);
	void Walk(float d);
	void UpDown(float d);

	void Pitch(float angle); //x rot
	void Yaw(float angle);	//y rot
	void Roll(float angle);	//z rot

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);

	DirectX::XMFLOAT4X4 WorldViewProj();

	void SetPos(DirectX::XMFLOAT3 pos);
	void OnResize(double ratio);
private:
	void UpdateView();

private:

	DirectX::XMFLOAT3 m_pos;		//카메라 위치
	DirectX::XMFLOAT3 m_forward;	//카메라의 전방 벡터
	DirectX::XMFLOAT3 m_up;			//카메라의 상향
	DirectX::XMFLOAT3 m_right;		//카메라의 우측


	float m_nearZ;
	float m_farZ;

	float m_FOV;

	float m_nearWindowHeight;
	float m_farWindowHeight;
	float m_ratio;

	DirectX::XMMATRIX m_worldMT;
	DirectX::XMMATRIX m_viewMT;
	DirectX::XMMATRIX m_projMT;
	DirectX::XMMATRIX m_worldviewprojMT;

	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT4X4 m_worldviewproj;
	DirectX::XMFLOAT4X4 m_orthoProj;

private:
	InputManager* m_input;
	float m_moveSpeed;

};

