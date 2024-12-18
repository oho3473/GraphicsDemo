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

	const float scale = 5.0;	// 샘플링 범위를 확장하여 더 두꺼운 엣지를 얻음
	const float2 texelSize = float2(scale / Width, scale / Height);
	const float2 offsetCoord[9] = {
		float2(-1, -1), float2(0, -1), float2(1, -1),
		float2(-1,  0), float2(0,  0), float2(1,  0),
		float2(-1,  1), float2(0,  1), float2(1,  1)
	};
	const float3 grayScale = float3(0.3, 0.59, 0.11);

	float edgeStrength = 0;  // 엣지 강도 계산

	for (int i = 0; i < 9; i++)
	{
		const float2 offset = offsetCoord[i] * texelSize;
		const float4 sampleColor = ObjectMask.Sample(LinearClamp, input.TexCoord + offset);

		// 필터 결과를 밝기로 계산하여 결과 누적 (RGB 대신 밝기 값 사용)
		edgeStrength += gMask[i] * dot(sampleColor.rgb, grayScale);  // 밝기 기준
	}
	
	// 엣지 감지 여부 결정 (임계값 기반)
	float isEdge = step(0.3, abs(edgeStrength));

	// 엣지가 감지된 경우 텍스처의 색상 사용, 그렇지 않으면 검정색
	if (isEdge > 0.0)
		output = ObjectMask.Sample(LinearClamp, input.TexCoord);
	else
		output = float4(0, 0, 0, 0);

	return output;
}

// 여기서 발생했던 문제
/* mask 변수를 전역 변수로 선언하고 사용할 때와 지역 변수로 선언하고 사용할때의 결과값이 다른 문제가 있었다.
 *
 * 원인 1
 * "HLSL 전역 변수의 초기화 문제"
 * HLSL에서 전역 변수로 배열을 선언할 때, 특히 상수 배열을 선언하면 컴파일러가 최적화 과정에서
 * 상수 데이터를 메모리에 저장하는 방식이 변경될 수 있다. 이로 인해 런타임시 값이 예상치 못한 방식으로 변경되거나
 * 불안정하게 초기화될 수 있다.
 *
 * 원인 2
 * "전역 변수의 메모리 배치 및 읽기 방식 차이"
 * HLSL에서 전역 변수는 상수 버퍼(Constant Buffer)에 저장되거나 메모리의 특정 위치에 고정될 수 있다.
 * 반면에, 지역 변수는 레지스터에 임시로 저장된다. 이러한 차이로 인해 필터링 연산에서의 값이 다르게 나타날 수 있다.
 *
 * -> 상수버퍼나 정적 상수 선언을 사용
 */
