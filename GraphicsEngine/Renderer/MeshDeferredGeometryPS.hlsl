///deferred rendering을 할때 offscreen buffer를 그려내는 shader

cbuffer EditColor : register(b0)
{
    float4 color;
}


cbuffer EditMaterial : register(b1)
{
    float4 editMaterial; //useEdit,Metalic,Roughness
};

//물체를 그려낼때 어떤 texture를 사용하는가?
cbuffer Material : register(b2)
{
    float4 useAMRO;    //Albedo, Metalic, Roughness, AO 
    float4 useNEOL; //normal, Emissive, Opacity, LightMap
    
    //외부에서 임의로 지정한 값을 사용하기위한 변수
    float4 albedo;  
    float metalness;    
    float roughness;
    float ao; // Ambient Occlusion
    float pad;
    
    //유니티에서 가져온 lightmap을 사용하기 위한 정보
    float4 lightmapdata; //index, offset(x,y),scale
    float2 lightmaptiling; // x y 라이트맵에서 해당 texture가 어떤 scale로 저장되어있는 정도
};

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

SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 posWorld : TEXCOORD0;
    float4 color : COLOR;
    float4 normal : TEXCOORD1;
    float4 tangent : TEXCOORD2;
    float4 bitangent : TEXCOORD3;
    float2 tex : TEXCOORD4;
    float2 lightuv : TEXCOORD5;
};

struct PS_OUTPUT
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Position : SV_Target2;
    float4 Depth : SV_Target3;
    float4 Metalic : SV_Target4;
    float4 Roughness : SV_Target5;
    float4 LightMap : SV_Target6;
    float4 Emissive : SV_Target7;
};

PS_OUTPUT main(VS_OUTPUT input)     // 출력 구조체에서 이미 Semantic 을 사용하고 있으므로 한번 더 지정해줄 필요는 없다.
{
    PS_OUTPUT output;
    output.Albedo = float4(0, 0, 0, 1);
    output.Normal = float4(0, 0, 0, 1);
    output.Position = float4(0, 0, 0, 1);
    output.Depth = float4(0, 0, 0, 1);
    output.Metalic = float4(0, 0, 0, 1);
    output.Roughness = float4(0, 0, 0, 1);
    output.LightMap = float4(0, 0, 0, 1);
    output.Emissive = float4(0, 0, 0, 1);


    output.Position = input.posWorld;

    float d = input.pos.z / input.pos.w;
    d *= 10;
    output.Depth = float4(1 - d, 1 - d, 1 - d, 1.0f);
    output.Albedo = input.color;

    
    
    if (editMaterial.x)
    {
        //output.Albedo.rgb = editMaterial.w;
        output.Albedo = (useAMRO.x * gAlbedo.Sample(samLinear, input.tex)) + ((1 - useAMRO.x) * input.color);
        output.Metalic.rgb = max(0.04, editMaterial.y);
        output.Roughness.rgb = max(0.04, editMaterial.z);
    }
    else
    {
        output.Albedo = (useAMRO.x * gAlbedo.Sample(samLinear, input.tex)) + ((1 - useAMRO.x) * input.color);
        output.Albedo.a = (useNEOL.w * gOpacity.Sample(samLinear, input.tex).r) + (1 - useNEOL.w);
        output.Metalic.rgb = max(0.04, useAMRO.y * gMetalic.Sample(samLinear, input.tex).b);
        output.Roughness.rgb = max(0, useAMRO.z * gRoughness.Sample(samLinear, input.tex).g);
    }
    
    output.Emissive = gEmissive.Sample(samLinear, input.tex) * useNEOL.y;

    output.Normal = input.normal;
    if (useNEOL.x >= 1)
    {
        float3 NormalTangentSpace = gNormal.Sample(samLinear, input.tex).rgb;
        NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
        NormalTangentSpace = normalize(NormalTangentSpace);

        float3x3 WorldTransform = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz); //면의 공간으로 옮기기위한 행렬
        output.Normal.xyz = normalize(mul(NormalTangentSpace, (WorldTransform)));
    }

    return output;
}