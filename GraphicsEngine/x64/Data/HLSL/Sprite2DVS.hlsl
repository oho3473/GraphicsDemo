cbuffer MatrixBuffer : register(b0)
{
	matrix gWorldMatrix;
	matrix gViewMatrix;
	matrix gProjectionMatrix;
}

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	input.position.w = 1.0f;

	output.position = mul(input.position, gWorldMatrix);
	output.position = mul(output.position, gViewMatrix);
	output.position = mul(output.position, gProjectionMatrix);

	output.uv = input.uv;

	return output;
}