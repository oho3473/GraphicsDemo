#include "Common.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float d = input.pos.z / input.pos.w;
    d *= 10;
    return float4(1 - d, 1 - d, 1 - d, 1.0f);
}