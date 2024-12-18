
#include "Common.hlsli"


float3x3 ExtractRotationMatrix(float4x4 worldMatrix)
{
    // ȸ�� ����� �ʱ�ȭ
    float3x3 rotationMatrix;

    // �����ϸ� ���� ����
    float3 scale;
    scale.x = length(float3(worldMatrix[0][0], worldMatrix[0][1], worldMatrix[0][2]));
    scale.y = length(float3(worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2]));
    scale.z = length(float3(worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2]));

    // ȸ�� ��� ��� (�����ϸ��� ������ ȸ��)
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
    
    
        //Unity light map�� ���ϴ��� 0,0 dx11 �»���� 0,0
        //y ���� �ʿ�
        float2 uv = float2(x, 1 - y);
    
        output.lightuv = uv;
        //output.LightMap = (gLightMap.Sample(samLinear, uv));
    }
    
    
    if (useNEOL.x > 0)
    {
        float3x3 meshWorld = ExtractRotationMatrix(gWorldInverse); //�޽��� ���� ����
        
        //���� vector
        float3 vTangent = normalize(mul((output.tangent.xyz), meshWorld));
        float3 vBitangent = normalize(mul((output.bitangent.xyz), meshWorld));
        float3 vNormal = normalize(mul((output.normal.xyz), meshWorld));
                        
        //���� ǥ���� ���� ������ ���� w == 1
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
    
    
    //Bone�� world
    output.posWorld = mul(mul(input.pos, skinning), gWorld);
    
    output.pos = mul(output.posWorld, gWorldViewProj);
    
    
    // �� ����� ���� ��, ���� 3x3 ȸ�� ������ ����
    float4x4 combinedMatrix = mul(skinning, gWorld);

    // float4x4 ��Ŀ��� ���� 3x3 ȸ�� ����� ����
    float3x3 rotationMatrix;
    rotationMatrix[0] = combinedMatrix[0].xyz;  // 1��
    rotationMatrix[1] = combinedMatrix[1].xyz;  // 2��
    rotationMatrix[2] = combinedMatrix[2].xyz;  // 3��
    
    //float3x3 rot = ExtractRotationMatrix(transpose(inverse(mul(skinning), gWorld))); 
    
    output.normal.xyz = normalize(mul(input.normal.xyz, rotationMatrix));
    output.tangent.xyz = normalize(mul(input.tangent.xyz, rotationMatrix));
    output.bitangent.xyz = normalize(mul(input.bitangent.xyz, rotationMatrix));
#endif     
    
    

    
    
    return output;
}
