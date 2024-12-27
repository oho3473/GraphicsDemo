
//PI
static const float Pi = 3.141592;
static const float Epsilon = 0.00001;

//모든 유전체에 대한 일정한 수직 입사 프레넬 계수
static const float3 Fdielectric = { 0.04, 0.04, 0.04 };
static const float gamma = 2.2;
//***********************************************
// Constant Buffers                             *
//***********************************************
//Camera
cbuffer Camera : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
};

//Transform
cbuffer Transform : register(b1)
{
    float4x4 gWorld;
    float4x4 gLocal;
    float4x4 gWorldInverse;
    float4x4 gLocalInverse;
};


//Material
cbuffer Material : register(b2)
{
    float4 AMRO;
    float4 useNEOL; //normal, Emissive, opacity, LightMap
    float4 albedo;
    float metalness;
    float roughness;
    float ao; // Ambient Occlusion
    float pad; 
    float4 lightmapdata; //index, offset(x,y),scale
    float2 lightmaptiling; // x y
};


//Light
struct LightData
{
    float3 Direction;
    float Range;

    float3 Attenuation;
    float pad2;
  
    float3 pos;
    float spot;
    
    float3 Color;
    float Intensity;
};

cbuffer LightArray : register(b3)
{
    LightData array[100];
    float DirIndex;
    float PointIndex;
    float SpotIndex;
    float pad4;
};

///Skinning
cbuffer MatrixPallete : register(b4)
{
    matrix MatrixPallete[128];
}

//TEXTURE
Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gPosition : register(t2);
Texture2D gDepth : register(t3);
Texture2D gMetalic : register(t4);
Texture2D gRoughness : register(t5);
Texture2D gAO : register(t6);
Texture2D gEmissive : register(t7);

Texture2D gOpacity : register(t8);
Texture2D gLightMap : register(t9);
Texture2D gGBuffer : register(t10);
Texture2D gIMGUI : register(t11);

//***********************************************
// Sampler States                               *
//***********************************************
SamplerState samLinear : register(s0);

//***********************************************
// Structures                                   *
//***********************************************

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
 #ifdef SKINNING
    float4 boneindex[2] : BONEINDEX;
    float4 boneweight[2] : BONEWEIGHT;
 #endif
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 posWorld : WORLDPOSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 tex : TEXCOORD;
    float2 lightuv : LIGHTMAPUV;
};

struct Quad
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct Particle
{
	float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW : SIZE;
    float Age : AGE;
    uint Type : TYPE;
};

//***********************************************
// LIGHT HELPER FUNCTIONS                    *
//***********************************************


float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float Calc_D(float3 N, float3 H, float roughness)
{
    //DistributionGGX
    float a = roughness * roughness;
    float a2 = a * a;
    
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    
    float den = Pi * denom * denom;

    return num / den;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    //GeometrySchlickGGX
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float Calc_G(float3 N, float3 V, float3 L, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float3 H = normalize(L + V);
    
    //GeometrySmith[
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float NdotH = saturate(dot(N, H));
        
    float ggx1 = GeometrySchlickGGX(NdotL, k); 
    float ggx2 = GeometrySchlickGGX(NdotV, k);

    return ggx1 * ggx2;
}

float3 CalcDir(LightData lightData, float3 V, float3 N, float3 F0,float3 albedo, float roughness, float metalicValue)
{
    float3 result  = float3(0,0,0);
    
    float3 ambient  = float3(0,0,0);
    float3 diffuse  = float3(0,0,0);
    float3 specular = float3(0,0,0);
    
    //표면점에서 광원으로의 벡터 
    float3 L = -normalize(lightData.Direction); //directionlight는 모든 표면점에서 일정한 방향으로 들어오는 빛이므로 빛의 방향을 역으로 쓰자
    float3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    float3 F = FresnelSchlick(F0, max(0.0, dot(H,V))); //최소값 F0 , 최대값은 1.0,1.0,1.0
    
    //Diffuse BRDF
    //kD - diffuse 반사율, kS - fresnel 반사율 -> 에너지 보존 법칙에 의해 프레넬로 반사되는 빛의 양과 물체에 흡수되 표면 밑에서 산란해 반사되는 빛의 양은 1
    float3 kD = float3(1.0, 1.0, 1.0) - F;//    +float3(0.4, 0.4, 0.4); // kS is equal to Fresnel
    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting, or a linear blend if partly metal (pure metals have no diffuse light)
    kD *= (1.0 - metalicValue);
    diffuse = kD * albedo * Pi;
   
    
    //Specular BRDF
    
    float D = Calc_D(N, H, max(0.01, roughness));
    float G = Calc_G(N, V, L, max(0.01, roughness));
    
    float3 n = (F * D * G); //분자
    float3 d = 4.0 * NdotV * NdotL + 0.01; //분모 0으로 나누기 피하려고 + 0.01
    
    specular = n / d;
    
    
    result = (specular + diffuse) * lightData.Intensity * lightData.Color * NdotL;
    
    return result;
}

float3 CalcPoint(LightData lightData,float4 pos , float3 V, float3 N, float3 F0, float3 albedo, float roughness, float metalicValue, float3 Depth)
{
    float3 result  = float3(0,0,0);
        
    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);
    
    float3 L = (lightData.pos - pos.xyz); //표면에서 광원까지
    
    //광원과 표면의 거리
    float distance = length(L);
    
    if (distance > lightData.Range)
        return float3(0, 0, 0);
    
    //빛 벡터 정규화
    L /= distance;
    
    float3 H = normalize(L + V);
    float NdotL = dot(N, L); //max(dot(N, L), 0.0); //이거 문제
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
            
    float3 F = FresnelSchlick(F0, max(0.0, dot(H,V))); //최소값 F0 , 최대값은 1.0,1.0,1.0
    
    //Diffuse BRDF
    //kD - diffuse 반사율, kS - fresnel 반사율 -> 에너지 보존 법칙에 의해 프레넬로 반사되는 빛의 양과 물체에 흡수되 표면 밑에서 산란해 반사되는 빛의 양은 1
    float3 kD = float3(1.0, 1.0, 1.0) - F; // kS is equal to Fresnel
    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting, or a linear blend if partly metal (pure metals have no diffuse light)
    kD *= 1.0 - metalicValue;
    
    diffuse = kD * albedo * Pi;
    
   //Specular BRDF
    
    float D = Calc_D(N, H, max(0.01, roughness));
    float G = Calc_G(N, V, L, max(0.01, roughness));
    
    float3 n = (F * D * G); //분자
    float3 d = 4.0 * NdotV * NdotL + 0.01; //분모 0으로 나누기 피하려고 + 0.01
    
    specular = n / d;
        
     // 감쇠: 부드럽게 처리된 감쇠 계산
    float smoothAtt = smoothstep(0, 1.0, lightData.Range - distance); //x가 [min, max] 범위에 있는 경우 0과 1 사이의 매끄러운 Hermite 보간을 반환합 (min,max,x)

    
    //float att = lightData.Intensity / dot(lightData.Attenuation, float3(1.0f, distance, distance * distance));
    float att = lightData.Intensity / (lightData.Attenuation.x + lightData.Attenuation.y * distance + lightData.Attenuation.z * distance * distance);
    
    att *= smoothAtt;
    float3 radiance = lightData.Color * att;
    
    result += (specular + diffuse) * radiance /*radiance 복사-(빛날)휘도*/ * max(dot(N, L), 0.0);    
   return result;    
}

float3 CalcSpot(LightData lightData, float4 pos, float3 V, float3 N, float3 F0, float3 albedo, float roughness, float metalicValue)
{
    float3 result  = float3(0,0,0);
    
    
    float3 ambient = float3(0, 0, 0);
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);
    
    //빛 벡터
    float3 L = lightData.pos - pos;
    
    //광원과 표면의 거리
    float distance = length(L);
    
    if (distance > lightData.Range)
        return float3(0,0,0);
    
    //빛 벡터 정규화
    L /= distance;
    float3 H = normalize(L + V);
    
    float3 F = FresnelSchlick(F0, max(0.0, dot(H,V))); //최소값 F0 , 최대값은 1.0,1.0,1.0
    
   //Diffuse BRDF
    //kD - diffuse 반사율, kS - fresnel 반사율 -> 에너지 보존 법칙에 의해 프레넬로 반사되는 빛의 양과 물체에 흡수되 표면 밑에서 산란해 반사되는 빛의 양은 1
    float3 kD = float3(1.0, 1.0, 1.0) - F; // kS is equal to Fresnel
    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting, or a linear blend if partly metal (pure metals have no diffuse light)
    kD *= 1.0 - metalicValue;
    
    diffuse = kD * albedo / Pi;
    
 
   //Specular BRDF
    
    float D = Calc_D(N, H, roughness);
    float G = Calc_G(N, V, L, roughness);
    
    float3 n = (F * D * G); //분자
    float3 d = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.01; //분모 0으로 나누기 피하려고 + 0.01
    
    specular = n / d;

    float spot = pow(max(dot(-L, lightData.Direction), 0.0f), lightData.spot);

    float att = spot / dot(lightData.Attenuation, float3(1.0f, distance, distance * distance));
    
    diffuse *= att;
    specular *= att;
    
    result += (specular + diffuse) * lightData.Color/*radiance 복사-(빛날)휘도*/ * max(dot(N, L), 0.0) * lightData.Intensity;
    return result;
}

//***********************************************
// PARTICLE HELPER FUNCTIONS                    *
//***********************************************

