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

	//필수
	uint32_t EntityID;
	std::wstring Name;
	std::wstring FBX;
	int ModelID;

	bool isSkinned = false;
	bool isVisible = true;

	DirectX::SimpleMath::Matrix local; //캐릭터 자체 로컬
	DirectX::SimpleMath::Matrix world; //게임 세상의 위치
	DirectX::SimpleMath::Vector3 rotation;

	//overdraw
	bool isOverDraw = false;	//벽뒤에 플레이어랑 총 짤리는거 방지 flag
	bool punchEffect;	//펀치 이펙트 메쉬 flag

	//애니메이션
	float duration;
	float preDuration;
	bool isPlay;
	int curAni;
	int preAni;
	float transitionDuration;

	//디버그박스용 - 나중에 지울거
	std::wstring textureName;

	//라이트맵 데이터
	DirectX::SimpleMath::Vector2 offset;
	DirectX::SimpleMath::Vector2 tiling;
	int lightmapindex;
	float scale;

	// 아웃라인 관련 변수
	DirectX::SimpleMath::Color MaskingColor = DirectX::SimpleMath::Color{ 0, 0, 0, 0 };
	// 인터랙티브 COLOR
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