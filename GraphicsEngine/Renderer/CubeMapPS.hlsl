

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 normal : TEXCOORD0;
    float4 tangent : TEXCOORD1;
    float4 bitangent : TEXCOORD2;
    float4 tex : TEXCOORD3; //uv,lightmapindex,uselightmapflag
    float2 lightuv : TEXCOORD4;
    float4 posWorld : TEXCOORD5;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}