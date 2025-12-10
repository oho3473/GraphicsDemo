Texture2D Gbuffer: register(t0);
Texture2D SSAO : register(t1);
SamplerState samLinear : register(s0);

cbuffer useSSAO : register(b0)
{
    float4 useSSAO;
}

struct Quad
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 posWorld : TEXCOORD0;
    float4 color : COLOR;
    float4 normal : TEXCOORD1;
    float4 tangent : TEXCOORD2;
    float4 bitangent : TEXCOORD3;
    float2 tex : TEXCOORD4;
    float2 lightuv : TEXCOORD5;
};


float4 main(VS_OUTPUT input) : SV_TARGET
{
    
    return float4(1, 1, 1, 1);
}