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
	float leftPercent;   // ���ʿ��� ������� ����
	float rightPercent;  // �����ʿ��� ������� ����
	float topPercent;    // ���ʿ��� ������� ����
	float bottomPercent; // �Ʒ��ʿ��� ������� ����
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float2 uv = input.uv;

// ���ʿ��� ������� �κ� ���
if (input.uv.x < leftPercent) discard;

// �����ʿ��� ������� �κ� ���
if (input.uv.x > (1.0f - rightPercent)) discard;

// ���ʿ��� ������� �κ� ���
if (input.uv.y < topPercent) discard;

// �Ʒ��ʿ��� ������� �κ� ���
if (input.uv.y > (1.0f - bottomPercent)) discard;

	// �ؽ�ó ���� ���ø�
	float4 color = shaderTexture.Sample(SampleType, uv);

	// gColor�� ����
	color *= gColor;

	return color;
}