#include"Common.hlsli"

//TEXTURE
Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gPosition : register(t2);
Texture2D gDepth : register(t3);
Texture2D gMetalic : register(t4);
Texture2D gRoughness : register(t5);
Texture2D gAO : register(t6);
Texture2D gEmissive : register(t7);

Texture2D gOpacity : register(t8);
Texture2D gLightMap : register(t9);
Texture2D gGBuffer : register(t10);
TextureCube gIrradiance : register(t11); //
TextureCube gRadiance : register(t12); //
Texture2D gLUT : register(t13); //

SamplerState samLinear : register(s0);


cbuffer useIBL : register(b5)
{
    float4 isIBL;
}



float4 main(VS_OUTPUT input) : SV_Target
{
    float3 Fresnel = float3(0, 0, 0);
    float3 Distribute = float3(0, 0, 0);
    float3 GeometryAttenuation = float3(0, 0, 0);
    float3 Specular = float3(0, 0, 0);
    float3 Diffuse = float3(0, 0, 0);
	
    float4 position = gPosition.Sample(samLinear, input.tex);
    float4 N = gNormal.Sample(samLinear, input.tex);

    float3 result = float3(0, 0, 0);
    float3 directlight = float3(0, 0, 0);
    float3 indirectlight = float3(0, 0, 0);

	//View
    float3 V = float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - position;

	//texture sampling
    float3 albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction 여기서도 해주자
    float opacity = (1 - useNEOL.z) + (useNEOL.z * gOpacity.Sample(samLinear, input.tex).r); //투명도
    float metallicValue = gMetalic.Sample(samLinear, input.tex).b;
    float roughnessValue = gRoughness.Sample(samLinear, input.tex).g;
	
    float3 EmissiveValue = pow(gEmissive.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma));
    float4 depthTemp = gDepth.Sample(samLinear, input.tex);
    float3 Depth = float3(depthTemp.x, depthTemp.y, depthTemp.z);


    float3 cubemap = gLightMap.Sample(samLinear, input.tex).rgb;
	
	//cube texture의 정보를 얻어온다
    uint width, height, levels;
    gRadiance.GetDimensions(0, width, height, levels);
	
    float3 irradiance = gIrradiance.Sample(samLinear, N.xyz);
    float3 reflection = reflect(-V, N); //시선이 점에 들어가 반사되는 벡터를 통해 환경맵에서 어떤 값을 가져올지 확인
    float3 radiance = gRadiance.SampleLevel(samLinear, reflection, roughnessValue * levels); //거칠기에 따른 밉맵 적용
    float2 DF = gLUT.Sample(samLinear, float2(max(0,dot(N, normalize(V))), roughness));
	
    if (isIBL.r)
    {
		//indirectlight = CalcIBL(V, N.xyz, F0, albedoColor, roughnessValue, metallicValue, irradiance, radiance);
    }
	
	
    for (int i = 0; i < DirIndex; i++)
    {
        directlight += CalcDir(array[i], V, N.xyz, albedoColor, roughnessValue, metallicValue, Fresnel, Distribute, GeometryAttenuation, Diffuse, Specular);
    }
    
    // Calculate Spot Light    
    for (int k = DirIndex; k < DirIndex + SpotIndex; k++)
    {
	    //directlight += CalcSpot(array[k], position, V, N.xyz, F0, albedoColor, roughnessValue, metallicValue, Depth);
    }
    
    // Calculate Point Light
    for (int j = DirIndex + SpotIndex; j < DirIndex + SpotIndex + PointIndex; j++)
    {
        directlight += CalcPoint(array[j], position, V, N.xyz, albedoColor, roughnessValue, metallicValue, Fresnel, Distribute, GeometryAttenuation, Diffuse, Specular);
    }

    result = directlight + indirectlight;

    result = result + EmissiveValue + cubemap;

   // gamma correct
    result = pow(result, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));


   
	
    return float4(result,1);
}