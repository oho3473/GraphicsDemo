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

struct PS_OUTPUT
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Position : SV_Target2;
    float4 Depth : SV_Target3;
    float4 Metalic_Roughness : SV_Target4;
    float4 AO : SV_Target5;
    float4 LightMap : SV_Target6;
    float4 Emissive : SV_Target7;
};

cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

//Transform
cbuffer Transform : register(b1)
{
    float4x4 gWorld;
    float4x4 gLocal;
    float4x4 gWorldInverse;
    float4x4 gLocalInverse;
};


cbuffer time : register(b2)
{
    float4 deltaTime;
}

Texture2D normalTex : register(t0);
Texture2D noiseTex : register(t1);


SamplerState samLinear : register(s0);




float4 main(VS_OUTPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    //noise texture * pow((1 - saturate(n dot V) ,0.2) * sin(6.23f * (연출효과 + (1 - (2 * (1 - saturate(n dot V)) ))))
    
    
    float3 NormalTangentSpace = normalTex.Sample(samLinear, input.tex).rgb;
    NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
    NormalTangentSpace = normalize(NormalTangentSpace);

    float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
    
    //최종 normal
    float3 N = normalize(mul(NormalTangentSpace, (WorldTransform)));  
    
     //View
    float3 V = normalize(float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - input.posWorld.xyz);
    
    float effect = 1 - frac(deltaTime.r);
    //effect = 0.1;
    
    float TAU = 6.23f;
    
    
    float2 noiseUV = float2((effect * 0.1f) + (input.tex.x), input.tex.y);
    
    
    float noiseValue = noiseTex.Sample(samLinear, noiseUV);
    
    
    float4 color = noiseValue * pow(1 - saturate(dot(N, V)), 1.5) * sin( TAU * (effect + (1 - (2 * pow(1 - saturate(dot(N, V)),1)))));
   
    if (color.w <= 0.1)
    {
       discard;
    }
    
    //return float4(noiseValue, 0, 0, 1);
    color = float4(0.227, 0.745, 0.576, color.w);
    
    return color;
    
    //return float4(N.xyz, 1);
       
}