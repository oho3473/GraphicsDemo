#include "Common.hlsli"

/// <summary>
///
/// The pixel shader for the geometry pass in the first pass of "Deferred Shading".
///
/// </summary>




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

PS_OUTPUT main(VS_OUTPUT input)     // 출력 구조체에서 이미 Semantic 을 사용하고 있으므로 한번 더 지정해줄 필요는 없다.
{
    PS_OUTPUT output;
    output.Position = input.posWorld;

    float d = input.pos.z / input.pos.w;
    d *= 10;
    output.Depth = float4(1 - d, 1 - d, 1 - d, 1.0f);
    output.Albedo = input.color;
       
    if (AMRO.x > 0)
    {
        output.Albedo = gAlbedo.Sample(samLinear, input.tex);
    }
    
    output.Metalic_Roughness = float4(0, 0, 0, 1);
    output.Metalic_Roughness.r = 0.04f;
    if (AMRO.y >= 1)
    {
        output.Metalic_Roughness.r = gMetalic.Sample(samLinear, input.tex).r;
    }
    
    output.Metalic_Roughness.g = 1.f;
    
    if (AMRO.z >= 1)
    {
        output.Metalic_Roughness.g = gRoughness.Sample(samLinear, input.tex).r;
    }
    
    output.AO = 0.f;
    
    if (AMRO.w >= 1)
    {
        output.AO = gAO.Sample(samLinear, input.tex);
    }
    
    output.Normal = input.normal;
    if (useNEOL.x >= 1)
    {
        float3 NormalTangentSpace = gNormal.Sample(samLinear, input.tex).rgb;
        NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
        NormalTangentSpace = normalize(NormalTangentSpace);

        float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
        output.Normal.xyz = normalize(mul(NormalTangentSpace, (WorldTransform)));
    }
	
    output.Emissive = 0;
    if (useNEOL.y >= 1)
    {
        output.Emissive = gEmissive.Sample(samLinear, input.tex);
    }
    
    if(useNEOL.z >= 1)
    {
        output.Albedo.a = gOpacity.Sample(samLinear, input.tex).r;
    }
        
    output.LightMap = gLightMap.Sample(samLinear, input.lightuv);
    
    return output;
    
}