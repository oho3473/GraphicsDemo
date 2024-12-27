
#include"Common.hlsli"

struct PS_OUTPUT
{
    float4 Gbuffer : SV_Target0;
};

float4 calcindirect()
{
    //for (int i = 0; i < sampleCount; ++i)
    //{
    //// Generate sample direction
    //    float3 sampleDir = normalize(float3(rand() - 0.5, rand() - 0.5, rand() - 0.5));
    
    //// Project sample point
    //    float3 samplePos = position + sampleDir * radius;
    
    //// Fetch the indirect lighting contribution from the sample point
    //    float3 sampleColor = gAlbedo.Sample(samLinear, samplePos.xy).rgb;
    //    float3 sampleNormal = normalize(gNormal.Sample(samLinear, samplePos.xy).xyz);
    
    //// Simple Lambertian reflection model for indirect light
    //    float NdotL = max(dot(normal, sampleDir), 0.0);
    //    indirectLight += sampleColor * NdotL;
    //}
    
    return float4(0,0,0,0);

}


float4 main(VS_OUTPUT input) : SV_TARGET
{
    //PS_OUTPUT output;
    
    float opacity = 1.0f;
    if (useNEOL.z >= 1)
    {
        opacity = gOpacity.Sample(samLinear, input.tex).r;
    }
    
    float4 position = gPosition.Sample(samLinear, input.tex);
    float4 N = gNormal.Sample(samLinear, input.tex);
    
    float3 result = float3(0, 0, 0);
    float3 directlight = float3(0, 0, 0);
    float3 indirectlight = float3(0, 0, 0);
    
    //View
    float3 V = normalize(float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - position.xyz);
    
    //texture sampling
    float3 albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction 여기서도 해주자
    float metallicValue = gMetalic.Sample(samLinear, input.tex).r; //quad에 텍스처를 샘플링해서 이상하게 나오는거였음
    float roughnessValue = gMetalic.Sample(samLinear, input.tex).g;
    float aoValue = gAO.Sample(samLinear, input.tex).r;
    float3 EmissiveValue = pow(gEmissive.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma));
    float4 depthTemp = gDepth.Sample(samLinear, input.tex);
    float3 Depth = float3(depthTemp.x, depthTemp.y, depthTemp.z);
    
        
     //수직 입사 시의 반사율 - 비금속이면 0.04 금속이면 metalic RGB 언리얼4는 이렇게 쓴다
    float3 F0 = Fdielectric;
    F0 = lerp(F0, albedoColor, max(0.04, metallicValue));
    
    for (int i = 0; i < DirIndex; i++)
    {
        directlight += CalcDir(array[i], V, N.xyz, F0, albedoColor, roughnessValue, metallicValue);
    }

    // Calculate Spot Light    
    for (int k = DirIndex; k < DirIndex + SpotIndex; k++)
    {
        directlight += CalcSpot(array[k], position, V, N.xyz, F0, albedoColor, roughnessValue, metallicValue);
    }
    // Calculate Point Light
    for (int j = DirIndex + SpotIndex; j < DirIndex + SpotIndex + PointIndex; j++)
    {
        directlight += CalcPoint(array[j], position, V, N.xyz, F0, albedoColor, roughnessValue, metallicValue, Depth);

    }
    
    float2 uv = gLightMap.Sample(samLinear, input.tex).rg;
    
    indirectlight = float3(0,0,0);
    indirectlight = pow(gLightMap.Sample(samLinear, input.tex), float3(gamma, gamma, gamma));

    //ambient lighting (constant factor for simplicity)
    float3 ambient = aoValue * albedoColor;

    // directlight = indirectlight + directlight;
 
    result = directlight + indirectlight;
    
    result = result + EmissiveValue;
    
    // gamma correct
    result = pow(result, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));
    
    //output.Gbuffer = float4(result, 1);
    
    //return output;
    return float4(result, 1);
    //return float4(albedoColor, 1);
}