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