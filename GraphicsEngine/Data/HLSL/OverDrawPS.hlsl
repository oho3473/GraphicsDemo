#include "Common.hlsli"

struct PS_OUTPUT
{
    float4 GBuffer : SV_Target0;
    float4 Normal : SV_Target1;
};

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
    
    float3 albedoColor;
     
    float metallicValue;
    float roughnessValue;
    float aoValue;
    float opacity = 1.0f;
    //수직 입사 시의 반사율 - 비금속이면 0.04 금속이면 metalic RGB 언리얼4는 이렇게 쓴다
    float3 F0 = Fdielectric;
    
     //// Calculate Directional Light 
 
    float4 albedo = pow(gAlbedo.Sample(samLinear, input.tex), gamma);
    float3 Depth = gDepth.Sample(samLinear, input.tex);
    
    float3 result = float3(0, 0, 0);
    float4 emissive;
    
    //View
    float3 V = normalize(float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - input.posWorld.xyz);
   
    //tangentspace를 계산해 normal을 만든다
    float4x4 meshWorld = gWorldInverse;
    float4 vTangent = (input.tangent);
    float4 vBitangent = (input.bitangent);
    float4 vNormal = (input.normal);
        
    float3 NormalTangentSpace = gNormal.Sample(samLinear, input.tex).rgb; //-1~1
    NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
    
    //float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
    float3x3 WorldTransform = float3x3(vTangent.xyz, vBitangent.xyz, vNormal.xyz); //면의 공간으로 옮기기위한 행렬
        
    float4 N;
    N.xyz = normalize(mul(NormalTangentSpace, WorldTransform));
    output.Normal = N;
    /*
    */
  
    if (AMRO.x > 0)
    {
        albedoColor = pow(gAlbedo.Sample(samLinear, input.tex), gamma).rgb;
    }
    else
    {
        albedoColor = input.color;
    }
    
    if (AMRO.y > 0)
    {
        metallicValue = gMetalic.Sample(samLinear, input.tex).r;
        F0 = lerp(F0, albedoColor, metallicValue);
        
    }
    else
    {
        metallicValue = 0.04f; //비금속이면 0.04 금속이면 metalic RGB 언리얼4는 이렇게 쓴다
    }
    
    if (AMRO.z > 0)
    {
        roughnessValue = gMetalic.Sample(samLinear, input.tex).g;
    }
    else
    {
        roughnessValue = 1;
    }
    
    if (AMRO.w > 0)
    {
        aoValue = gAO.Sample(samLinear, input.tex).r;
    }
    else
    {
        aoValue = 0;
    }
        
    if (useNEOL.y > 0)
    {
        emissive = gEmissive.Sample(samLinear, input.tex);
        
    }
    else
    {
        emissive = 0;
    }
    
    if (useNEOL.z > 0)
    {
        opacity = gOpacity.Sample(samLinear, input.tex);
    }
    
    for (int i = 0; i < DirIndex; i++)
    {
        result += CalcDir(array[i], V, N.xyz, F0, albedoColor, roughnessValue, metallicValue);
    }

    // Calculate Spot Light    
    for (int k = DirIndex; k < SpotIndex; k++)
    {
        result += CalcSpot(array[k], input.posWorld, V, N.xyz, F0, albedoColor, roughnessValue, metallicValue);
    }
    // Calculate Point Light
    for (int j = DirIndex + SpotIndex; j < DirIndex + SpotIndex + PointIndex; j++)
    {
        result += CalcPoint(array[j], input.posWorld, V, N.xyz, F0, albedoColor, roughnessValue, metallicValue, Depth);

    }
    
    float3 lightColor = float3(1.0, 1.0, 1.0); // Assuming white light

    // ambient lighting (constant factor for simplicity)
    float3 ambient = aoValue * albedoColor;

    float3 color = ambient + result;

    
    color = color + emissive;
    
    // gamma correct
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
       
    output.GBuffer = float4(color, 1);
    return output;

}