#pragma once

#ifdef RENDERDATA_EXPORTS
#define RENDERDATA_API __declspec(dllexport)
#else
#define RENDERDATA_API __declspec(dllimport)
#endif

#include "SimpleMath.h"
#include <string>


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

	DirectX::SimpleMath::Matrix local; //ĳ���� ��ü ����
	DirectX::SimpleMath::Matrix world; //���� ������ ��ġ
	DirectX::SimpleMath::Vector3 rotation;

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
	DirectX::SimpleMath::Vector2 offset;
	DirectX::SimpleMath::Vector2 tiling;
	int lightmapindex;
	float scale;

	// �ƿ����� ���� ����
	DirectX::SimpleMath::Color MaskingColor = DirectX::SimpleMath::Color{ 0, 0, 0, 0 };
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