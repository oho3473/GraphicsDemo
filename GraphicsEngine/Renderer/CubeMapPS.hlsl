

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : TEXCOORD0;
    float4 tangent : TEXCOORD1;
    float4 bitangent : TEXCOORD2;
    float4 tex : TEXCOORD3; //uv,lightmapindex,uselightmapflag
    float2 lightuv : TEXCOORD4;
    float4 posWorld : TEXCOORD5;
};

SamplerState samLinear : register(s0);

Texture2D CubeTex : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return CubeTex.Sample(samLinear, input.tex.xy);
}