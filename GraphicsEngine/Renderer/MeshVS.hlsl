
#include "Common.hlsli"


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
        
    output.posWorld = mul(input.pos, gWorld);
    output.pos = mul(output.posWorld, gWorldViewProj);
    output.color = input.color;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.bitangent = input.bitangent;
    output.tex = input.tex;
    output.lightuv = input.lightuv;
        
    if (useNEOL.w > 0)
    {
        float x = (input.lightuv.x * lightmaptiling.x) + lightmapdata.x;
        float y = (1 - input.lightuv.y) * lightmaptiling.y + lightmapdata.y;
    
    
        //Unity light map은 좌하단이 0,0 dx11 좌상단이 0,0
        //y 반전 필요
        float2 uv = float2(x, 1 - y);
    
        output.lightuv = uv;
        //output.LightMap = (gLightMap.Sample(samLinear, uv));
    }
    
    
    if (useNEOL.x > 0)
    {
        float3x3 meshWorld = ExtractRotationMatrix(gWorldInverse); //메쉬의 월드 공간
        
        //방향 vector
        float3 vTangent = normalize(mul((output.tangent.xyz), meshWorld));
        float3 vBitangent = normalize(mul((output.bitangent.xyz), meshWorld));
        float3 vNormal = normalize(mul((output.normal.xyz), meshWorld));
                        
        //색상 표현을 위해 점으로 저장 w == 1
        output.normal = float4(vNormal.xyz, 1);
        output.tangent = float4(vTangent.xyz, 0);
        output.bitangent = float4(vBitangent.xyz, 0);
    }
    
    
#ifdef SKINNING
    int index[8];
    index[0] = input.boneindex[0].x;
    index[1] = input.boneindex[0].y;
    index[2] = input.boneindex[0].z;
    index[3] = input.boneindex[0].w;
    
    index[4] = input.boneindex[1].x;
    index[5] = input.boneindex[1].y;
    index[6] = input.boneindex[1].z;
    index[7] = input.boneindex[1].w;
    
    
    float weight[8];
    weight[0] = input.boneweight[0].x;
    weight[1] = input.boneweight[0].y;
    weight[2] = input.boneweight[0].z;
    weight[3] = input.boneweight[0].w;
    
    weight[4] = input.boneweight[1].x;
    weight[5] = input.boneweight[1].y;
    weight[6] = input.boneweight[1].z;
    weight[7] = input.boneweight[1].w;
    
    float4x4 skinning;
    
    skinning =  mul(saturate(weight[0]), MatrixPallete[index[0]]);
    skinning += mul(saturate(weight[1]), MatrixPallete[index[1]]);
    skinning += mul(saturate(weight[2]), MatrixPallete[index[2]]);
    skinning += mul(saturate(weight[3]), MatrixPallete[index[3]]);
    skinning += mul(saturate(weight[4]), MatrixPallete[index[4]]);
    skinning += mul(saturate(weight[5]), MatrixPallete[index[5]]);
    skinning += mul(saturate(weight[6]), MatrixPallete[index[6]]);
    skinning += mul(saturate(weight[7]), MatrixPallete[index[7]]);
    
    
    //Bone의 world
    output.posWorld = mul(mul(input.pos, skinning), gWorld);
    
    output.pos = mul(output.posWorld, gWorldViewProj);
    
    
    // 두 행렬을 곱한 후, 상위 3x3 회전 성분을 추출
    float4x4 combinedMatrix = mul(skinning, gWorld);

    // float4x4 행렬에서 상위 3x3 회전 행렬을 추출
    float3x3 rotationMatrix;
    rotationMatrix[0] = combinedMatrix[0].xyz;  // 1행
    rotationMatrix[1] = combinedMatrix[1].xyz;  // 2행
    rotationMatrix[2] = combinedMatrix[2].xyz;  // 3행
    
    //float3x3 rot = ExtractRotationMatrix(transpose(inverse(mul(skinning), gWorld))); 
    
    output.normal.xyz = normalize(mul(input.normal.xyz, rotationMatrix));
    output.tangent.xyz = normalize(mul(input.tangent.xyz, rotationMatrix));
    output.bitangent.xyz = normalize(mul(input.bitangent.xyz, rotationMatrix));
#endif     
    
    

    
    
    return output;
}
