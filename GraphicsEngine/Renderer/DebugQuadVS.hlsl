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

struct Quad
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD;
};


cbuffer quadtransform : register(b0)
{
    float4x4 world;
};

VS_OUTPUT main(Quad input)
{
    VS_OUTPUT test;
     
    test.pos = mul(input.pos,world);
    test.tex = input.tex;

    return test;
}