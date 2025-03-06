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
    float3 V = float3(gViewInverse._41, gViewInverse._42, gViewInverse._43) - position.xyz;

	//texture sampling
    float3 albedoColor = pow(gAlbedo.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma)); //Gamma Correction ���⼭�� ������
    float opacity = (1 - useNEOL.z) + (useNEOL.z * gOpacity.Sample(samLinear, input.tex).r); //����
    float metallicValue = gMetalic.Sample(samLinear, input.tex).b;
    float roughnessValue = gRoughness.Sample(samLinear, input.tex).g;
    float3 EmissiveValue = pow(gEmissive.Sample(samLinear, input.tex).rgb, float3(gamma, gamma, gamma));
	
	//�ֺ� ȯ���� ��ü�� ��ŭ ���ߴ����� ���� ����� texture���� �����´� (������ ��հ�) 
    float3 irradiance = gIrradiance.Sample(samLinear, N.xyz); //diffuse�� ������ �ش�. ȯ����� �����ų� �ֺ� ���� ���ȭ�� texture
    
	//cube texture�� ������ ���´�
    uint width, height, levels;
    gRadiance.GetDimensions(0, width, height, levels);
    float3 reflection = reflect(-V, N); //�ü��� ���� �� �ݻ�Ǵ� ���͸� ���� ȯ��ʿ��� � ���� �������� Ȯ��
    //�ü��� ���� �ֺ����� �������� �κ��� �� ���� �������� ���� ������ texture�� �����´�
    float3 radiance = gRadiance.SampleLevel(samLinear, reflection, roughnessValue * levels); //��ĥ�⿡ ���� �Ӹ� ����
    
    //Scale : �ݻ縦 ��ŭ�� ����� �ϴ°�, Bias : 
    float2 scaleBias = gLUT.Sample(samLinear, float2(max(0,dot(N.xyz, normalize(V))), roughnessValue)); //���ɻ��� ������ �������� ���� texture�� �������� �װ��� �����;���
	
    if (isIBL.r)
    {
        indirectlight = CalcIBL(V, N.xyz, albedoColor, roughnessValue, metallicValue, irradiance, radiance, scaleBias);
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


   // gamma correct
    result = pow(result, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));
	
    return float4(result,1);
}