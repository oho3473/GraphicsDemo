struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

Texture2D outLine : register(t0);
Texture2D offScreen : register(t1);

SamplerState defaultSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float3 output = float3(0, 0, 0);

	//output += outLine.Sample(defaultSampler, input.TexCoord);
	//output += offScreen.Sample(defaultSampler, input.TexCoord);

	output = outLine.Sample(defaultSampler, input.TexCoord);
	//output = offScreen.Sample(defaultSampler, input.TexCoord);

	return float4(output, 1);
}

float4 add(VS_OUTPUT input) : SV_TARGET
{
	float4 output = float4(0, 0, 0, 1);

	output += outLine.Sample(defaultSampler, input.TexCoord);
	output += offScreen.Sample(defaultSampler, input.TexCoord);

	return output;
}