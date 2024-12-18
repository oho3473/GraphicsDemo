struct VS_OUTPUT
{
    float4 pos : SV_POSITION; //screen space
    float4 posWorld : TEXCOORD0;
    float4 posClip : TEXCOORD1;
    float4 decalInverse0 : TEXCOORD2;
    float4 decalInverse1 : TEXCOORD3;
    float4 decalInverse2 : TEXCOORD4;
    float4 decalInverse3 : TEXCOORD5;
};

struct PS_OUTPUT
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
};

cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

cbuffer RenderTargetSize: register(b1)
{
    float4 width_height;
}

Texture2D Position : register (t0);
Texture2D Normal : register (t1);
Texture2D Decal : register (t2);
//Texture2D DecalNormal : register (t3);


SamplerState samLinear : register(s0);

float4 main(VS_OUTPUT input) : SV_Target
{
    
    PS_OUTPUT output;
    
    float2 screenPos = input.posClip.xy / input.posClip.w;
    float2 Pos_UV = screenPos * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    float3 worldPos = Position.Sample(samLinear,Pos_UV);
    float3 normal = Normal.Sample(samLinear, Pos_UV);
    
    float4x4 inversedecal;
    inversedecal[0] = input.decalInverse0;
    inversedecal[1] = input.decalInverse1;
    inversedecal[2] = input.decalInverse2;
    inversedecal[3] = input.decalInverse3;
        
    float4 posInDecal = mul(float4(worldPos, 1), inversedecal);
    
    if (-0.5f <= posInDecal.x && posInDecal.x <= 0.5f &&
        -0.5f <= posInDecal.y && posInDecal.y <= 0.5f &&
        -0.5f <= posInDecal.z && posInDecal.z <= 0.5f)
    {
         clip(0.5 - abs(posInDecal.xyz));
        
        float2 textureCoordinate;
        if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z))           
        {
            textureCoordinate = posInDecal.xz + 0.5;
        }
        else if (abs(normal.z) > abs(normal.x) && abs(normal.z) > abs(normal.y))
        {
            textureCoordinate = posInDecal.xy + 0.5;
        }
        else
        {
            textureCoordinate = posInDecal.yz + 0.5;
        }
        
        //float3 decalNormal = DecalNormal.Sample(samLinear, textureCoordinate).xyz * 2.0f - 1.0f;

        //decalNormal = normalize(decalNormal);

    // 3. Decal normal을 물체의 normal 방향에 맞춰 회전
    // worldSurfaceNormal과 decalNormal의 dot product 계산
        //float alignmentFactor = dot(normal, decalNormal);
        //if (alignmentFactor < 0.0)
        //{
        //    decalNormal = -decalNormal; // 방향이 반대인 경우 반전
        //}

    // 4. 최종적으로 정렬된 decal normal을 사용하여 조명 계산
        //float3 finalNormal = normalize(decalNormal + normal);
        
        return Decal.Sample(samLinear, textureCoordinate);
        
        //output.Albedo = Decal.Sample(samLinear, textureCoordinate);
        //output.Normal = float4(finalNormal, 1);
        //return output;
    }
    
    discard;
    return float4(0, 0, 0, 1);
    //return output;
}