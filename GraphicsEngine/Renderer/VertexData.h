#pragma once
#include <d3d11.h>
#include "BufferData.h"

struct BaseVertex
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT4 bitangent;
	DirectX::XMFLOAT2 TexCord;
	DirectX::XMFLOAT2 LightMapUV;
};

struct SkinningVertex
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT4 bitangent;
	DirectX::XMFLOAT2 TexCord;
	DirectX::XMFLOAT2 LightMapUV;
	float BoneIndices[8];
	float BoneWeights[8];
};

struct QuadVertex
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT2 TexCord;
};

//¿ŒΩ∫≈œΩÃ
struct InstanceData
{
	InstanceData() : world(DirectX::SimpleMath::Matrix::Identity),worldInverse(DirectX::SimpleMath::Matrix::Identity)
	{

	}
	/*
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT4 bitangent;
	DirectX::XMFLOAT2 TexCord;
	DirectX::XMFLOAT2 LightMapUV;
	*/

	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix worldInverse;
	DirectX::XMFLOAT2 lightmap_offset;
	DirectX::XMFLOAT2 lightmap_tiling;
	DirectX::XMFLOAT2 lightmap_index;
};


struct InstanceSkinnedData
{
	InstanceSkinnedData() : world(DirectX::SimpleMath::Matrix::Identity), worldInverse(DirectX::SimpleMath::Matrix::Identity)
	{

	}

	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix worldInverse;
};



struct DecalVertex
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tex;
};

//¿ŒΩ∫≈œΩÃ
struct InstanceDecalData
{
	InstanceDecalData() : world(DirectX::SimpleMath::Matrix::Identity), worldInverse(DirectX::SimpleMath::Matrix::Identity)
	{

	}

	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix worldInverse;
};
