cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : TEXCOORD0;
    float4 tangent : TEXCOORD1;
    float4 bitangent : TEXCOORD2;
    float4 tex : TEXCOORD3;
    float2 lightuv : TEXCOORD4;
    float4 posWorld : TEXCOORD5;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.pos = mul(input.pos, gWorldViewProj);

	return output;
}