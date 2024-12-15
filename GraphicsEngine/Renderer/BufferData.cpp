#include "BufferData.h"

TransformData::TransformData() : world(DirectX::SimpleMath::Matrix::Identity), local(DirectX::SimpleMath::Matrix::Identity),
localInverse(DirectX::SimpleMath::Matrix::Identity), worldInverse(DirectX::SimpleMath::Matrix::Identity)
{

}

LightData::LightData() : direction(0, 0, 0), range(-1), attenuation(0, 0, 0), type(-1), pos(0, 0, 0), spot(-1), color(1, 1, 1), intensity(1)
{

}

LightArray::LightArray() : DirIndex(0), PointIndex(0), SpotIndex(0), pad(0)
{
}

MaterialData::MaterialData() : albedo(1, 1, 1, 1), metalness(0), roughness(0), ao(0), useNEOL(0, 0, 0, 0), useAMRO(0, 0, 0, 0)
{

}
