struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Pos = input.Pos;
	output.TexCoord = input.TexCoord;

	return output;
}