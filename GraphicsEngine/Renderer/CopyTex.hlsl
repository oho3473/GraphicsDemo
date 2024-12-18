
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 posWorld : WORLDPOSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
};

Texture2D gTex : register(t0);
SamplerState samLinear : register(s0);


float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = gTex.Sample(samLinear, input.tex);
    return color;
}