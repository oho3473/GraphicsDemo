SamplerState LinearClamp : register(s0);
Texture2D ObjectMask : register(t0);

cbuffer ScreenSize : register(b0)
{
	unsigned int Width;
	unsigned int Height;
}

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

static const float gMask[9] =
{
	-1, -1, -1,
	-1,  8, -1,
	-1, -1, -1
};	// Laplacian Filter

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 output = float4(0, 0, 0, 1);

	const float scale = 5.0;	// ���ø� ������ Ȯ���Ͽ� �� �β��� ������ ����
	const float2 texelSize = float2(scale / Width, scale / Height);
	const float2 offsetCoord[9] = {
		float2(-1, -1), float2(0, -1), float2(1, -1),
		float2(-1,  0), float2(0,  0), float2(1,  0),
		float2(-1,  1), float2(0,  1), float2(1,  1)
	};
	const float3 grayScale = float3(0.3, 0.59, 0.11);

	float edgeStrength = 0;  // ���� ���� ���

	for (int i = 0; i < 9; i++)
	{
		const float2 offset = offsetCoord[i] * texelSize;
		const float4 sampleColor = ObjectMask.Sample(LinearClamp, input.TexCoord + offset);

		// ���� ����� ���� ����Ͽ� ��� ���� (RGB ��� ��� �� ���)
		edgeStrength += gMask[i] * dot(sampleColor.rgb, grayScale);  // ��� ����
	}
	
	// ���� ���� ���� ���� (�Ӱ谪 ���)
	float isEdge = step(0.3, abs(edgeStrength));

	// ������ ������ ��� �ؽ�ó�� ���� ���, �׷��� ������ ������
	if (isEdge > 0.0)
		output = ObjectMask.Sample(LinearClamp, input.TexCoord);
	else
		output = float4(0, 0, 0, 0);

	return output;
}

// ���⼭ �߻��ߴ� ����
/* mask ������ ���� ������ �����ϰ� ����� ���� ���� ������ �����ϰ� ����Ҷ��� ������� �ٸ� ������ �־���.
 *
 * ���� 1
 * "HLSL ���� ������ �ʱ�ȭ ����"
 * HLSL���� ���� ������ �迭�� ������ ��, Ư�� ��� �迭�� �����ϸ� �����Ϸ��� ����ȭ ��������
 * ��� �����͸� �޸𸮿� �����ϴ� ����� ����� �� �ִ�. �̷� ���� ��Ÿ�ӽ� ���� ����ġ ���� ������� ����ǰų�
 * �Ҿ����ϰ� �ʱ�ȭ�� �� �ִ�.
 *
 * ���� 2
 * "���� ������ �޸� ��ġ �� �б� ��� ����"
 * HLSL���� ���� ������ ��� ����(Constant Buffer)�� ����ǰų� �޸��� Ư�� ��ġ�� ������ �� �ִ�.
 * �ݸ鿡, ���� ������ �������Ϳ� �ӽ÷� ����ȴ�. �̷��� ���̷� ���� ���͸� ���꿡���� ���� �ٸ��� ��Ÿ�� �� �ִ�.
 *
 * -> ������۳� ���� ��� ������ ���
 */
