
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
    
    //instancing data
    float4x4 world : WORLD;
    float4x4 worldinverse : WORLDINVERSE;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;   //screen space
    float4 posWorld : TEXCOORD0;
    float4 posClip : TEXCOORD1;
    float4 decalInverse0 : TEXCOORD2;
    float4 decalInverse1 : TEXCOORD3;
    float4 decalInverse2 : TEXCOORD4;
    float4 decalInverse3 : TEXCOORD5;
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
            
    output.posWorld = mul(input.pos, input.world);
    output.posClip = mul(output.posWorld, gWorldViewProj);
    output.pos = output.posClip;
    
    float4 maxBox = float4(0.5f , 0.5f , 0.5f, 1);
    float4 minBox = float4(-0.5f , -0.5f , -0.5f, 1);
    
 
    
    
    output.decalInverse0 = input.worldinverse[0];
    output.decalInverse1 = input.worldinverse[1];
    output.decalInverse2 = input.worldinverse[2];
    output.decalInverse3 = input.worldinverse[3];
    
    
    return output;
}