#include "Common.hlsli"

cbuffer EditColor : register(b5)
{
    float4 editAlbedo; //x : useEditAlbedo, y : R, z : G, w : B
}


cbuffer EditMaterial : register(b6)
{
    float4 editMaterial; //useEditMetalic, useEditRoughness ,Metalic,Roughness
};

cbuffer useIBL : register(b7)
{
    float4 isIBL;
}

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
    
    float metallicValue;
    float roughnessValue;
    float3 albedoColor;
    
  
    
	//texture sampling
    if(editAlbedo.x)
    {
        albedoColor.rgb = editAlbedo.yzw;
    }
    else
    {
        albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction 여기서도 해주자
    }
    
    float opacity = (1 - useNEOL.z) + (useNEOL.z * gOpacity.Sample(samLinear, input.tex).r); //투명도
    
    if(editMaterial.x)
    {
        metallicValue = max(0.04, editMaterial.z);
    }
    else
    {
        metallicValue = max(0.04, gMetalic.Sample(samLinear, input.tex).b);
    }
    
    if (editMaterial.y)
    {
        roughnessValue = editMaterial.w;
    }
    else
    {
        roughnessValue = gRoughness.Sample(samLinear, input.tex).g;
    }
    
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
   
    //주변 환경이 물체를 얼마큼 비추는지에 대한 계수를 texture에서 가져온다 (조도의 평균값) 
    float3 irradiance = gIrradiance.Sample(samLinear, N.xyz); //diffuse에 영향을 준다. 환경맵을 뭉개거나 주변 값을 평균화한 texture
    
	//cube texture의 정보를 얻어온다
    uint width, height, levels;
    gRadiance.GetDimensions(0, width, height, levels);
    float3 reflection = reflect(-V, N); //시선이 점에 들어가 반사되는 벡터를 통해 환경맵에서 어떤 값을 가져올지 확인
    //시선에 따라 주변에서 비춰지는 부분이 내 눈에 들어는지에 대한 정보를 texture로 가져온다
    float3 radiance = gRadiance.SampleLevel(samLinear, reflection, roughnessValue * levels); //거칠기에 따른 밉맵 적용
    
    //Scale : 반사를 얼만큼의 세기로 하는가, Bias : 
    float2 scaleBias = gLUT.Sample(samLinear, float2(max(0, dot(N, normalize(V))), roughness)); //성능상의 이유로 적분항의 값을 texture로 저장했음 그값을 가져와야함
	
    if (isIBL.r)
    {
        Diffuse = Diffuse + IBLDiffuse(V, N.xyz, albedoColor, roughnessValue, metallicValue, irradiance, radiance, scaleBias);
        Specular = Specular + IBLSpecular(V, N.xyz, albedoColor, roughnessValue, metallicValue, irradiance, radiance, scaleBias);
    }
    
    output.Fresnel = float4(Fresnel, 1);
    output.Distribute = float4(Distribute, 1);
    output.GeometryAttenuation = float4(GeometryAttenuation, 1);
    output.Specular = float4(Specular, 1);
    output.Diffuse = float4(Diffuse, 1);
   
    return output;
}