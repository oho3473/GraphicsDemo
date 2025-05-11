
Texture2D DepthBuffer : register(t0);
Texture2D NormalBuffer : register(t1);
Texture2D PositionBuffer : register(t2);
Texture2D GBuffer : register(t3);


SamplerState samLinear : register(s0);


cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};





float4 main() : SV_TARGET
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}