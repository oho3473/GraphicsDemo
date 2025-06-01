
Texture2D DepthBuffer : register(t0);
Texture2D NormalBuffer : register(t1);
Texture2D PositionBuffer : register(t2);
Texture2D NoiseTex : register(t3);

SamplerState samLinear : register(s0);


cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

cbuffer Kernels : register(b1)
{
    float4 SSAOKernel[32];
}
    
cbuffer Noise : register(b2)
{
    float4 Noise[16];
}

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


float4 main(VS_OUTPUT input) : SV_Target
{
    float2 uv = input.tex;
    
    float3 pos = PositionBuffer.Sample(samLinear, uv);
    float4 viewPos = mul(gViewInverse, float4(pos.xyz, 1));
    float4 normal = NormalBuffer.Sample(samLinear, uv);
    
    float3x3 viewspace = gViewInverse;
    float3 viewNormal = mul(viewspace, normal.xyz);
    
    float4 useDepth = DepthBuffer.Sample(samLinear, uv);
    float4 depth = viewPos.z;

    float occlusion = 0;
    float ao = 1;
    
    float kernelSize = 32;

    if (useDepth.w) //depth.w 유효 플래그로 사용중
    {

        //반구 영역의 회전을 위해 nosie랑 normal을 곱해 방향 변환
        float2 NoiseScale = (1920.0 / 16.0, 1080.0 / 16.0); //screen width / noise size, screen height / noise size
        float3 randomVec = normalize(NoiseTex.Sample(samLinear, uv * NoiseScale).xyz);
    
        // TBN matrix (tangent, bitangent, normal)
        float3 tangent = normalize(cross(randomVec, viewNormal));
        float3 bitangent = cross(viewNormal, tangent);
        float3x3 TBN = float3x3(tangent, bitangent, viewNormal);
    
        for (int i = 0; i < kernelSize; ++i)
        {
            float radius = 1.0f;
        // Sample in tangent space and transform to view space
            float3 sampleVec = mul(TBN, SSAOKernel[i].xyz);
            float3 samplePos = viewPos.xyz + sampleVec * radius;

        // Project sample position back to screen space
            float4 offset = mul(gProj, float4(samplePos, 1.0f));
            offset.xyz /= offset.w;
            float2 sampleUV = offset.xy * 0.5f + 0.5f;

        // Sample depth at that screen 
            float4 samplePosition = PositionBuffer.Sample(samLinear, uv);
            float4 sampleViewPos = mul(gViewInverse, float4(samplePosition.xyz, 1));
            float3 sampleDepth = sampleViewPos.z;

        // If sample is behind the surface, count it as occluded
            float bias = 0.05;
            float rangeCheck = smoothstep(0.0f, 1.0f, 1 /*radius*/ / abs(viewPos.z - sampleDepth.z));
            if (sampleDepth.z >= samplePos.z + bias)
            {
                occlusion += rangeCheck;
            }
        }
    }

    ao = 1 - (occlusion/kernelSize);
    
    return float4(ao,ao,ao, 1.0);
}