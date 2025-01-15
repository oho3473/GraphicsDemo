#include "Common.hlsli"

struct PS_OUTPUT
{
    float4 Fresnel : SV_Target0;
    float4 Distribute : SV_Target1;
    float4 GeometryAttenuation : SV_Target2;
    float4 Specular : SV_Target3;
    float4 Diffuse : SV_Target4;
};

Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gPosition : register(t2);
Texture2D gMetalic : register(t4);
Texture2D gRoughness : register(t5);
Texture2D gOpacity : register(t8);
TextureCube gIrradiance : register(t11);
TextureCube gRadiance : register(t12); 
Texture2D gLUT : register(t13); 


SamplerState samLinear : register(s0);


PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
	
    float3 Fresnel = float3(0, 0, 0);
    float3 Distribute = float3(0, 0, 0);
    float3 GeometryAttenuation = float3(0, 0, 0);
    float3 Specular = float3(0, 0, 0);
    float3 Diffuse = float3(0, 0, 0);
    
	//texture sampling
    float3 albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction 여기서도 해주자
    float opacity = (1 - useNEOL.z) + (useNEOL.z * gOpacity.Sample(samLinear, input.tex).r); //투명도
    float metallicValue = gMetalic.Sample(samLinear, input.tex).b;
    float roughnessValue = gRoughness.Sample(samLinear, input.tex).g;
    float4 position = gPosition.Sample(samLinear, input.tex);
    float4 N = input.normal;
    if (useNEOL.x >= 1)
    {
        float3 NormalTangentSpace = gNormal.Sample(samLinear, input.tex).rgb;
        NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
        NormalTangentSpace = normalize(NormalTangentSpace);

        float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
        N.xyz = normalize(mul(NormalTangentSpace, (WorldTransform)));
    }
    //View
    float3 V = float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - input.posWorld;

    for (int i = 0; i < DirIndex; i++)
    {
        CalcDir(array[i], V, N.xyz, albedoColor, roughnessValue, metallicValue, Fresnel, Distribute, GeometryAttenuation, Diffuse, Specular);
    }
   
    output.Fresnel = float4(Fresnel, 1);
    output.Distribute = float4(Distribute, 1);
    output.GeometryAttenuation = float4(GeometryAttenuation, 1);
    output.Specular = float4(Specular, 1);
    output.Diffuse = float4(Diffuse, 1);
   
    return output;
}