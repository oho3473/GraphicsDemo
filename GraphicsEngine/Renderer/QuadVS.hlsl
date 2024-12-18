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

VS_OUTPUT main(Quad input)
{
    VS_OUTPUT test;
        
    test.pos = input.pos;
    //test.pos = input.pos;
    //test.posWorld = input.pos;
    test.tex = input.tex;
    
    return test;
}
