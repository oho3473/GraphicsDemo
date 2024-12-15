#pragma once
#include "SimpleMath.h"


//상수버퍼는 16바이트의 배수
//#define GRAPICS_EXPORT_API __declspec(dllexport)

struct __declspec(dllexport) TransformData
{
	TransformData();

	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix local;

	DirectX::SimpleMath::Matrix worldInverse;
	DirectX::SimpleMath::Matrix localInverse;
};

struct __declspec(dllexport) CameraData
{
	DirectX::SimpleMath::Matrix worldviewproj;
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
	DirectX::SimpleMath::Matrix viewInverse;
	DirectX::SimpleMath::Matrix projInverse;
	DirectX::SimpleMath::Matrix orthoProj;
};

static_assert(sizeof(CameraData) % 16 == 0, "must be align");

struct __declspec(dllexport) MatrixPallete
{
	DirectX::SimpleMath::Matrix offset[128];
};


enum class __declspec(dllexport) LightType
{
	Direction = 0,
	Spot,
	Point,

	End
};

struct __declspec(dllexport) LightData
{
	LightData();

	DirectX::XMFLOAT3 direction;
	float range;

	DirectX::XMFLOAT3 attenuation;
	float type;

	DirectX::XMFLOAT3 pos;
	float spot;

	DirectX::XMFLOAT3 color;
	float intensity;
};


struct __declspec(dllexport) LightArray
{
	LightArray();

	LightData array[100];
	float DirIndex;
	float PointIndex;
	float SpotIndex;
	float pad;
};

struct __declspec(dllexport) MaterialData
{
	MaterialData();

	DirectX::XMFLOAT4 useAMRO;	//텍스처를 가지고 있는가 albedo, metalic, roughness, AO
	DirectX::XMFLOAT4 useNEOL;	//텍스처를 가지고 있는가 normal, Emissive, opacity, LightMap
	DirectX::XMFLOAT4 albedo;
	float metalness;
	float roughness;
	float ao; // Ambient Occlusion
	float pad;
	DirectX::XMFLOAT4  lightmapdata; //index, offset(x,y),scale
	DirectX::XMFLOAT2  lightmaptiling; //index, offset(x,y),scale
	DirectX::XMFLOAT2  pad2;
};
