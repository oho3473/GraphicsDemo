

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 posWorld : WORLDPOSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
};

cbuffer TexelSize : register(b0)
{
    float2 screensize;
};

//edge detection
Texture2D gNormal : register(t0);
Texture2D gDepth : register(t1);


sampler gLinear : register(s0);



float gamma = 2.2f;


float4 main(VS_OUTPUT input) : SV_TARGET
{
    //Laplasian
    const float Filter[9] =
    {
        -1.f, -1.f, -1.f,
    -1.f, 8.f, -1.f,
    -1.f, -1.f, -1.f,
    };
    
    float2 texel = 1 / screensize;
    
    
    float3 normalCenter = gNormal.Sample(gLinear, input.tex);
    float3 normalLeft = gNormal.Sample(gLinear, input.tex + float2(-texel.x, 0)).xyz;
    float3 normalRight = gNormal.Sample(gLinear, input.tex + float2(texel.x, 0)).xyz;
    float3 normalUp = gNormal.Sample(gLinear, input.tex + float2(0, -texel.y)).xyz;
    float3 normalDown = gNormal.Sample(gLinear, input.tex + float2(0, texel.y)).xyz;
    
    float3 normalLeftUp = gNormal.Sample(gLinear, input.tex + float2(-texel.x, -texel.y)).xyz;
    float3 normalRightUp = gNormal.Sample(gLinear, input.tex + float2(texel.x, -texel.y)).xyz;
    float3 normalLeftDown = gNormal.Sample(gLinear, input.tex + float2(-texel.x, texel.y)).xyz;
    float3 normalRightDown = gNormal.Sample(gLinear, input.tex + float2(texel.x, texel.y)).xyz;
    
    
    float3 normalValue[9] =
    {
        normalLeftUp, normalUp, normalRightUp,
        normalLeft, normalCenter, normalRight,
        normalLeftDown, normalDown, normalRightDown,
    };
    
    float3 laplacianResult = float3(0, 0, 0);
    
    for (int i = 0; i < 9; i++)
    {
        laplacianResult += Filter[i] * normalValue[i];
    }
    
    

    // 결과 값의 크기 계산 (변화량 측정)
    float magnitude = length(laplacianResult);
    
    
    // 임계값 설정 (Normal 변화량이 크면 경계로 간주)
    float threshold = 1.0f;

    
    float4 output;
    // 경계를 감지하여 Outline 그리기
    if (magnitude > threshold)
    {
        output = float4(0, 0.466f, 0.529f, 1);
    }
    else
    {
        output = float4(0.0470f, 0.0f, 0.3490f, 1.0f);
    }

    //aces tone mapping    
    {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        //output = saturate((output * (a * output + b)) / (output * (c * output + d) + e));
    }
        
    return output;
}