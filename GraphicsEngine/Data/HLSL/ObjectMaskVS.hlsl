#include "Common.hlsli"

struct VertexOut
{
    float4 PosWorld : SV_POSITION;
    float4 Color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
};

VertexOut main(VS_INPUT input)
{
    VertexOut output;

    output.PosWorld = mul(input.pos, gWorld);
    output.PosWorld = mul(output.PosWorld, gWorldViewProj);
    output.Color = input.color;

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
    skinning = mul(saturate(weight[0]), MatrixPallete[index[0]]);
    skinning += mul(saturate(weight[1]), MatrixPallete[index[1]]);
    skinning += mul(saturate(weight[2]), MatrixPallete[index[2]]);
    skinning += mul(saturate(weight[3]), MatrixPallete[index[3]]);
    skinning += mul(saturate(weight[4]), MatrixPallete[index[4]]);
    skinning += mul(saturate(weight[5]), MatrixPallete[index[5]]);
    skinning += mul(saturate(weight[6]), MatrixPallete[index[6]]);
    skinning += mul(saturate(weight[7]), MatrixPallete[index[7]]);

    output.PosWorld = mul(mul(input.pos, skinning), gWorld);
    output.PosWorld = mul(output.PosWorld, gWorldViewProj);
#endif     
    
    
    return output;
}
