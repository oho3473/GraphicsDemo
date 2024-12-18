
cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};



//Material
cbuffer Material : register(b1)
{
    float4 AMRO;
    float4 useNEOL; //normal, Emissive, opacity, LightMap
    float4 albedo;
    float metalness;
    float roughness;
    float ao; // Ambient Occlusion
    float pad;
    float4 lightmapdata; //index, offset(x,y),scale
    float2 lightmaptiling; // x y
};


///Skinning
cbuffer MatrixPallete : register(b2)
{
    matrix MatrixPallete[128];
}

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
Texture2D gIMGUI : register(t11);

//***********************************************
// Sampler States                               *
//***********************************************
SamplerState samLinear : register(s0);

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
    
    //instancing data
    float4x4 world : WORLD;
    float4x4 worldinverse : WORLDINVERSE;
    float2 offset : LIGHTMAPOFFSET;
    float2 tiling : LIGHTMAPTILING;
    float2 index : LIGHTMAPINDEX; //index , uselightmap
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TEXCOORD0;
    float4 bitangent : TEXCOORD1;
    float4 tex : TEXCOORD2; //uv,lightmapindex,uselightmapflag
    float2 lightuv : TEXCOORD3;
    float4 posWorld : TEXCOORD4;
};


float3x3 ExtractRotationMatrix(float4x4 worldMatrix)
{
    // 회전 행렬을 초기화
    float3x3 rotationMatrix;

    // 스케일링 벡터 추출
    float3 scale;
    scale.x = length(float3(worldMatrix[0][0], worldMatrix[0][1], worldMatrix[0][2]));
    scale.y = length(float3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));
    scale.z = length(float3(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]));

    // 회전 행렬 계산 (스케일링을 제외한 회전)
    rotationMatrix[0][0] = worldMatrix[0][0] / scale.x;
    rotationMatrix[0][1] = worldMatrix[0][1] / scale.x;
    rotationMatrix[0][2] = worldMatrix[0][2] / scale.x;

    rotationMatrix[1][0] = worldMatrix[1][0] / scale.y;
    rotationMatrix[1][1] = worldMatrix[1][1] / scale.y;
    rotationMatrix[1][2] = worldMatrix[1][2] / scale.y;

    rotationMatrix[2][0] = worldMatrix[2][0] / scale.z;
    rotationMatrix[2][1] = worldMatrix[2][1] / scale.z;
    rotationMatrix[2][2] = worldMatrix[2][2] / scale.z;

    return rotationMatrix;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
            
    output.posWorld = mul(input.pos, input.world);
    output.pos = mul(output.posWorld, gWorldViewProj);
    
    output.color = input.color;
    output.tex.xy = input.tex.xy;
    output.tex.zw = input.index;
    
       
    float x = (input.lightuv.x * input.tiling.x) + input.offset.x;
    float y = (1 - input.lightuv.y) * input.tiling.y + input.offset.y;
    
    
    //Unity light map은 좌하단이 0,0 dx11 좌상단이 0,0
    //y 반전 필요
    float2 uv = float2(x, 1 - y);
    
    output.lightuv = min(uv, 1);
  
    
    

    float3x3 meshWorld = ExtractRotationMatrix(input.worldinverse); //메쉬의 월드 공간
        
        //방향 vector
    float3 vTangent =   useNEOL.x * normalize(mul((input.tangent.xyz), meshWorld))   + (1 - useNEOL.x) * input.tangent.xyz;
    float3 vBitangent = useNEOL.x * normalize(mul((input.bitangent.xyz), meshWorld)) + (1 - useNEOL.x) * input.bitangent.xyz;
    float3 vNormal =    useNEOL.x * normalize(mul((input.normal.xyz), meshWorld))    + (1 - useNEOL.x) * input.normal.xyz;
                        
        //색상 표현을 위해 점으로 저장 w == 1
    output.normal = float4(vNormal.xyz, 1);
    output.tangent = float4(vTangent.xyz, 0);
    output.bitangent = float4(vBitangent.xyz, 0);
        
        //float3 NormalTangentSpace = gNormal.SampleLevel(samLinear, input.tex,4).rgb;
        //NormalTangentSpace = NormalTangentSpace * 2.0f - 1.0f; //-1~1
        //NormalTangentSpace = normalize(NormalTangentSpace);
        
        //float3x3 WorldTransform = float3x3(vTangent.xyz, vBitangent.xyz, vNormal.xyz); //면의 공간으로 옮기기위한 행렬
        //float3 worldNormal = normalize(mul(NormalTangentSpace, WorldTransform));
        //output.normal = float4(worldNormal, 1);

    
    return output;
}