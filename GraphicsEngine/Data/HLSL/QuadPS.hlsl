#include "Common.hlsli"

Texture2D gGBuffer : register(t0);
SamplerState samLinear : register(s0);


float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = gGBuffer.Sample(samLinear, input.tex);
    return color;
}