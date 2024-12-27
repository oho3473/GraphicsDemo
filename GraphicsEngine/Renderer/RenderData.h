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

namespace debug
{
	struct __declspec(dllexport)SphereInfo
	{
		DirectX::BoundingSphere Sphere;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)AABBInfo	// AABB
	{
		DirectX::BoundingBox AABB;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)OBBInfo
	{
		DirectX::BoundingOrientedBox OBB;
		float xAxisAngle = 0.f;
		float yAxisAngle = 0.f;
		float zAxisAngle = 0.f;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)FrustumInfo
	{
		DirectX::BoundingFrustum Frustum;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)GridInfo
	{
		DirectX::SimpleMath::Vector3 XAxis;
		DirectX::SimpleMath::Vector3 YAxis;
		DirectX::SimpleMath::Vector3 Origin;	//중심점
		size_t XDivs;	//x몇칸
		size_t YDivs;	//y몇칸
		float GridSize;	//1칸의 길이
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)RingInfo
	{
		DirectX::SimpleMath::Vector3 Origin;
		DirectX::SimpleMath::Vector3 MajorAxis;
		DirectX::SimpleMath::Vector3 MinorAxis;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)TriangleInfo
	{
		DirectX::SimpleMath::Vector2 PointA;
		DirectX::SimpleMath::Vector2 PointB;
		DirectX::SimpleMath::Vector2 PointC;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport)QuadInfo
	{
		DirectX::SimpleMath::Vector2 PointA;
		DirectX::SimpleMath::Vector2 PointB;
		DirectX::SimpleMath::Vector2 PointC;
		DirectX::SimpleMath::Vector2 PointD;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};

	struct __declspec(dllexport) RayInfo
	{
		DirectX::SimpleMath::Vector3 Origin;
		DirectX::SimpleMath::Vector3 Direction;
		bool Normalize = true;
		DirectX::SimpleMath::Color Color = { 1.f, 1.f, 1.f, 1.f };
	};
}