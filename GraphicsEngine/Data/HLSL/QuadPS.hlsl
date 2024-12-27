#include "Common.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = gGBuffer.Sample(samLinear, input.tex);   	
    return color;
}