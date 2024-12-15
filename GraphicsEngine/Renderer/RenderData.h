#pragma once

#include "SimpleMath.h"
#include <string>



#ifdef RENDERDATA_EXPORTS
#define RENDERDATA_API __declspec(dllexport)
#else
#define RENDERDATA_API __declspec(dllimport)
#endif

using namespace DirectX::SimpleMath;

struct __declspec(dllexport) RenderData
{
public:
	RenderData();
	~RenderData();

	//�ʼ�
	uint32_t EntityID;
	std::wstring Name;
	std::wstring FBX;
	int ModelID;

	bool isSkinned = false;
	bool isVisible = true;

	Matrix local; //ĳ���� ��ü ����
	Matrix world; //���� ������ ��ġ
	Vector3 rotation;

	//overdraw
	bool isOverDraw = false;	//���ڿ� �÷��̾�� �� ©���°� ���� flag
	bool punchEffect;	//��ġ ����Ʈ �޽� flag

	//�ִϸ��̼�
	float duration;
	float preDuration;
	bool isPlay;
	int curAni;
	int preAni;
	float transitionDuration;

	//����׹ڽ��� - ���߿� �����
	std::wstring textureName;

	//����Ʈ�� ������
	Vector2 offset;
	Vector2 tiling;
	int lightmapindex;
	float scale;

	// �ƿ����� ���� ����
	Color MaskingColor = Color{ 0, 0, 0, 0 };
	// ���ͷ�Ƽ�� COLOR
	DirectX::XMFLOAT4 color;
};



namespace decal
{
	struct __declspec(dllexport) Info
	{
		std::string TexturePath;
		DirectX::SimpleMath::Matrix	WorldTransform;
	};
}