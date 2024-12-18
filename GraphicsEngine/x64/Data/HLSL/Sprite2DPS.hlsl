Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

cbuffer ColorBuffer : register(b0)
{
	float4 gColor;
	float leftPercent;   // 왼쪽에서 사라지는 비율
	float rightPercent;  // 오른쪽에서 사라지는 비율
	float topPercent;    // 위쪽에서 사라지는 비율
	float bottomPercent; // 아래쪽에서 사라지는 비율
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float2 uv = input.uv;

// 왼쪽에서 사라지는 부분 계산
if (input.uv.x < leftPercent) discard;

// 오른쪽에서 사라지는 부분 계산
if (input.uv.x > (1.0f - rightPercent)) discard;

// 위쪽에서 사라지는 부분 계산
if (input.uv.y < topPercent) discard;

// 아래쪽에서 사라지는 부분 계산
if (input.uv.y > (1.0f - bottomPercent)) discard;

	// 텍스처 색상 샘플링
	float4 color = shaderTexture.Sample(SampleType, uv);

	// gColor와 곱셈
	color *= gColor;

	return color;
}