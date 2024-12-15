#include "RenderData.h"

RenderData::RenderData() : EntityID(0), Name(L""), FBX(L""), ModelID(0),
	world(DirectX::SimpleMath::Matrix::Identity),
	local(DirectX::SimpleMath::Matrix::Identity),
	rotation(DirectX::SimpleMath::Vector3::Zero)
{
}

RenderData::~RenderData() 
{
}