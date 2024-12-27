#include "Common.hlsli"
    

cbuffer Color : register(b2)
{
    float4 color;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float d = input.pos.z / input.pos.w;
    d *= 10;
    
    if (color.w > 0)
    {
        float4 albedo = pow(gAlbedo.Sample(samLinear, input.tex), gamma);
        return albedo;
    }
    else
    {
        
        float4 lit;
        lit.x = input.color.x + color.x;
        lit.y = input.color.x + color.y;
        lit.z = input.color.x + color.z;
        lit.w = 1;
    
    
        return lit;
    }
    //return float4(0.f, 1.0f, 0.f, 0.f);
}