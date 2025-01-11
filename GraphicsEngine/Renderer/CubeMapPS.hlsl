
cbuffer mainCamera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

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

TextureCube CubeTex : register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    //view

     float3 V = normalize(float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - input.posWorld.xyz);


     float gamma = 2.2f;

     float4 result;
     result = float4(0, 0, 0, 1);
     result.xyz = CubeTex.Sample(samLinear, -V);
     
    //return CubeTex.Sample(samLinear, reflect(-V, input.normal));
    return result;
}