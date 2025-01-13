#include "RenderData.h"

RenderData::RenderData() : EntityID(0), Name(L""), FBX(L""), ModelID(0),
	world(DirectX::SimpleMath::Matrix::Identity),
	local(DirectX::SimpleMath::Matrix::Identity),
	rotation(DirectX::SimpleMath::Vector3::Zero), preAni(0),preDuration(0),scale(1),color(1,1,1,1),curAni(0),duration(0),isPlay(false),lightmapindex(0),transitionDuration(0)
{
}

RenderData::~RenderData() 
{
}