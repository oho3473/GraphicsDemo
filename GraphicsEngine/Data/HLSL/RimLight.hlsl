

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


//Texture2D gBuffer : register(t0);
Texture2D gNormal : register(t0);

sampler gLinear : register(s0);


cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

//Material
cbuffer Material : register(b1)
{
    float4 AMRO;
    float4 useNEOL; //normal, Emissive, opacity, LightMap
    float4 albedo;
    float metalness;
    float roughness;
    float ao; // Ambient Occlusion
    float pad;
    float4 lightmapdata; //index, offset(x,y),scale
    float2 lightmaptiling; // x y
};

cbuffer Color: register(b2)
{
    float4 color;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{   
    float4 baseColor = color;     
    
   
    float4 Normal = input.normal;
    if (useNEOL.x >= 1)
    {
        float3 NormalTangentSpace = gNormal.Sample(gLinear, input.tex).rgb;
        NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
        NormalTangentSpace = normalize(NormalTangentSpace);
        
        float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
        Normal.xyz = normalize(mul(NormalTangentSpace, (WorldTransform)));
    }
    

    float4 N = Normal;

    float3 V = normalize(float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - input.pos.xyz);
    float NdotV = saturate(dot(N.xyz, V));
    
    
    float rim = 1 - NdotV;
    rim = smoothstep(0, 1.0f, rim);
    
    float4 result = rim * baseColor;
    //result.w = 1; //우리는 림라이트로만 보여줄거라 alpha 1로 고정함
    return result;
}