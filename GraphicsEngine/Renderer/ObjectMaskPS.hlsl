cbuffer ColorBuffer : register(b0)
{
	float4 gColor;
}

struct VertexOut
{
    float4 PosWorld : SV_POSITION;
	float4 Color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;

};

struct PS_OUTPUT
{
    float4 ObjectMask : SV_Target0;
    float4 RimLightMask : SV_Target1;
};

Texture2D gNormal : register(t0);
SamplerState samLinear : register(s0);


PS_OUTPUT main(VertexOut input)
{
    PS_OUTPUT output;
	if(gColor.x > 0)
    {
        output.ObjectMask = gColor;
    }
    else
    {
        output.ObjectMask = 0;
    }
    
    
    
    float d = input.PosWorld.z / input.PosWorld.w;
    d *= 10;
    output.RimLightMask = float4(1 - d, 1 - d, 1 - d, 1.0f);
    
    //output.RimLightMask = input.PosWorld;
    //output.RimLightMask = input.PosWorld;
	
    return output;
  }
