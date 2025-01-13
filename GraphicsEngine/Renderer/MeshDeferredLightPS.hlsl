
#include"Common.hlsli"

cbuffer useIBL : register(b5)
{
	float4 isIBL;
}


struct PS_OUTPUT
{
	float4 Gbuffer : SV_Target0;
	float4 Fresnel : SV_Target1;
	float4 Distribute : SV_Target2;
	float4 Geometry : SV_Target3;
	float4 NdotL: SV_Target4;
};

PS_OUTPUT main(VS_OUTPUT input)
{
	PS_OUTPUT output;

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
	float3 albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction ���⼭�� ������
	float metallicValue = gMetalic.Sample(samLinear, input.tex).r; //quad�� �ؽ�ó�� ���ø��ؼ� �̻��ϰ� �����°ſ���
	float roughnessValue = gRoughness.Sample(samLinear, input.tex).r;
	float aoValue = gAO.Sample(samLinear, input.tex).r;
	float3 EmissiveValue = pow(gEmissive.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma));
	float4 depthTemp = gDepth.Sample(samLinear, input.tex);
	float3 Depth = float3(depthTemp.x, depthTemp.y, depthTemp.z);

	uint width, height, levels;
	gRadiance.GetDimensions(0,width,height, levels);

	float3 irradiance = gIrradiance.Sample(samLinear, N.xyz);
	float3 reflection = reflect(-V, N); //�ü��� ���� �� �ݻ�Ǵ� ���͸� ���� ȯ��ʿ��� � ���� �������� Ȯ��
	float3 radiance = gRadiance.SampleLevel(samLinear, reflection,roughnessValue * levels);

	//���� �Ի� ���� �ݻ��� - ��ݼ��̸� 0.04 �ݼ��̸� metalic RGB �𸮾�4�� �̷��� ����
	float3 F0 = Fdielectric;
	F0 = lerp(F0, albedoColor, metallicValue);

	if (isIBL.r)
	{

		indirectlight = CalcIBL(V, N.xyz, F0, albedoColor, roughnessValue, metallicValue, irradiance, radiance);
	}
	
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

   //float2 uv = gLightMap.Sample(samLinear, input.tex).rg;

   //indirectlight = pow(gLightMap.Sample(samLinear, input.tex), float3(gamma, gamma, gamma));

   //ambient lighting (constant factor for simplicity)
   float3 ambient = aoValue * albedoColor;

   // directlight = indirectlight + directlight;

   result = directlight + indirectlight;

   //float3 cubemap = pow(gLightMap.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma));
   float3 cubemap = gLightMap.Sample(samLinear, input.tex).rgb;
   result = result + EmissiveValue + cubemap;

   // gamma correct
   result = pow(result, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));

   //output.Gbuffer = float4(result, 1);

    output.Gbuffer = float4(result, 1);
    output.Fresnel = float4(1, 0, 0, 1);
    output.Distribute = float4(0, 1, 0, 1);
    output.Geometry= float4(0, 0, 1, 1);
    output.NdotL= float4(1, 1, 0, 1);
    return output;
	//return float4(result, 1);
}