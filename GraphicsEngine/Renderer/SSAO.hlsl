
Texture2D DepthBuffer : register(t0);
Texture2D NormalBuffer : register(t1);
Texture2D PositionBuffer : register(t2);
Texture2D NoiseTex : register(t3);

SamplerState samLinear : register(s0);


cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView; //cur camera pos in world
    float4x4 gProj;
    float4x4 gViewInverse; //view matrix
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
    float3x3 viewspace = gViewInverse;  //view matrix
     
    //get view pos
    float3 pos = PositionBuffer.Sample(samLinear, uv);  //world pos
    float4 viewPos = mul(float4(pos.xyz, 1),gViewInverse); // world * view
    
    //get view normal
    float4 normal = NormalBuffer.Sample(samLinear, uv);// world 
    float3 viewNormal = mul(normal.xyz,viewspace); //view normal
    
    //get cur pixel depth
    float4 useDepth = DepthBuffer.Sample(samLinear, uv); //world z / world w

    //result
    float occlusion = 0;
    float ao = 1;
        
    //sample count
    float kernelSize = 32;

    float debugValue;
    
    if (useDepth.w) //depth.w 유효 플래그로 사용중
    {
        float ScreenWidth = 1920.f;
        float ScreenHeight = 1080.f;
        float NoiseSize = 16.f;
        
        float radius = 20.0f;

        //반구 영역의 회전을 위해 nosie랑 normal을 곱해 방향 변환
        float2 NoiseScale = float2(ScreenWidth / NoiseSize, ScreenHeight / NoiseSize); //screen width / noise size, screen height / noise size
        float3 randomVec = normalize(NoiseTex.Sample(samLinear, uv * NoiseScale).xyz);
    
        // TBN matrix (tangent, bitangent, normal)
        float3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
        float3 bitangent = normalize(cross(viewNormal, tangent));
        float3x3 TBN = float3x3(tangent, bitangent, viewNormal);
        

        // TBN 행렬 시각화 출력 (R: Tangent, G: Bitangent, B: Normal)
        //float3 TBNColor = 0.5f * (float3(tangent.x, bitangent.x, normal.x) + 1.0f); // X축 시각화
        // 또는 각 벡터를 분리해서 보기
        // float3 TBNColor = 0.5f * (tangent + 1.0f);      // R: Tangent 방향만
        // float3 TBNColor = 0.5f * (bitangent + 1.0f);    // G: Bitangent 방향만
        // float3 TBNColor = 0.5f * (normal + 1.0f);       // B: Normal 방향만
        // return float4(TBNColor, 1.0f);
    
        for (int i = 0; i < kernelSize; ++i)
        {
            //회전된 반구에 점을 옮김
            float3 sampleVec = mul(SSAOKernel[i].xyz, TBN);
            //방향이 면의 뒤를 향하면 반대로 뒤집기
            float flip = sign(dot(sampleVec, viewNormal));
            
            //현재 viewspace상의 위치에서 반구영역에 존재하는 sampling 점 위치
            float3 samplePos = viewPos.xyz + flip * sampleVec * radius;

            //sampling point를 clip space상으로 옮겨 현재 화면상의 uv 얻어내기
            float4 samplePosInClip = mul(float4(samplePos, 1.0f), gProj);
            samplePosInClip.xyz /= samplePosInClip.w; //-1~1
            float2 sampleUV = samplePosInClip.xy * 0.5f + 0.5f; //0~1
            
            // 범위 벗어남 체크
            if (sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y > 1)
            {
                continue;
            }

            //position buffer 없이 view space의 pos 구하는 법
            float2 ndc = sampleUV * 2.0f - 1.0f; // [0,1] → [-1,1]
            float ClipZ = DepthBuffer.Sample(samLinear, sampleUV).x;
            float ClipW = DepthBuffer.Sample(samLinear, sampleUV).y;
            float depth = ClipZ / ClipW;
            float4 clipPos = float4(ndc.x * ClipW, ndc.y * ClipW, ClipZ, ClipW); // depth는 z/w (normalized z
            float4 realPos = mul(clipPos, gProjInverse); // 역투영
            realPos /= realPos.w;
            
            //화면상에 sampling된 점에 물체가 그려져있는가? 그려져있으면 정보를 가져온다
            //화면상에 그려진 실제 geometry의 정보
            float4 RealPosInWorld = PositionBuffer.Sample(samLinear, sampleUV);
            float3 RealPosInView = mul(RealPosInWorld.xyz, viewspace);
            
            // If sample is behind the surface, count it as occluded
            float bias = 0.05f;
            float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(viewPos.z - RealPosInView.z));
            
            //viewspace에서는 가까울수록 0으로 수렴 크면 카메라와 멀다
            //해당 위치에 geometry가 없으면 빛이 도달할 수 있다
            if ((RealPosInView.z - samplePos.z) > bias)
            {
                occlusion += rangeCheck;
            }
        }
    }
    else
    {
        return float4(1, 0, 0, 1);
    }
    
    //빛의 도달양 = 1 - 차폐량
    ao = 1 - (occlusion / kernelSize);
    
    //return saturate(pow(ao, 4));
    return float4(ao, ao, ao, 1.0);
}