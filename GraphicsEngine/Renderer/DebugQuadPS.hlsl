#include "Common.hlsli"

SamplerState samLinear : register(s0);
Texture2D gGBuffer : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = gGBuffer.Sample(samLinear, input.tex);
    return color;
}