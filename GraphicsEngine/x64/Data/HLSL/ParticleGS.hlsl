#define PT_EMITTER 0
#define PT_SIMULATER 1

#define SHAPE_CONE 0
#define SHAPE_SPHERE 1
#define SHAPE_BOX 2
#define SHAPE_CIRCLE 3

#define BILLBOARD 0
#define STRETCHED_BILLBOARD 1

#define PI 3.14159265359

cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewProj;
	float3 gEyePosW;
	float gGameTime;
	float gTimeStep;
};

cbuffer cbData : register(b1)
{
	float3 gEmitPosW;
	float3 gEmitDirW;

	float4 gStartColor;
	float4 gEndColor;

	float2 gStartSize;
	float gStartSpeed;
	float gDuration;
	int gIsLoop;

	int gRestart;
	float gStartLifetime;

	float gAngle;
	float gRadius;

	unsigned int gParticleShape;
	unsigned int gRenderMode;

	float gGravity;
}

Texture2DArray gTexArray : register(t0);
Texture1D gRandomTex : register(t1);
SamplerState samLinear : register(s0);

// 게임 시간과 직접 전달한 offset을 기반으로 랜덤 벡터를 샘플링해준다.
float3 RandUnitVec3(float offset)
{
	// 게임 시간 더하기 오프셋을 무작위 텍스처 추출을 위한
	// 텍스처 좌표로 사용한다.
	float u = (gGameTime + offset);

	// 벡터 성분들의 범위는 [-1, 1] 이다.
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	// 단위 구로 투영한다.
	return normalize(v);
}

void ProcessShape(float3 random, out float3 posW, out float3 velocity)
{
	if (gParticleShape == SHAPE_CIRCLE)
	{
		float2 randomDir = normalize(float2(random.x, random.z));
		float radius = gRadius * sqrt(saturate((random.y + 1.0f) * 0.5f));

		float2 pos = randomDir * radius;

		posW = gEmitPosW + float3(pos.x, 0.0f, pos.y);
		velocity = normalize(float3(randomDir.x, 0.0f, randomDir.y)) * gStartSpeed;
	}
	else if (gParticleShape == SHAPE_SPHERE)
	{
		const float3 randomDir = normalize(random);

		// 파티클의 위치를 구의 중심(gEmitPosW)에서 시작하도록 설정
		posW = gEmitPosW + randomDir * gRadius; // 구의 표면에서 시작

		// 속도 벡터를 randomDir을 기반으로 설정
		velocity = randomDir * gStartSpeed; // 구의 표면에서 바깥쪽으로 방출
	}
	else if (gParticleShape == SHAPE_BOX)
	{
		posW = gEmitPosW.xyz;
		velocity = gStartSpeed * random;
	}
	else if (gParticleShape == SHAPE_CONE)
	{
		// 원뿔의 높이와 밑면 반지름을 설정
		float coneHeight = gRadius; // 원뿔의 높이
		float baseRadius = coneHeight * tan(radians(gAngle)); // 밑면의 반지름 계산 (radians 함수로 각도를 라디안 값으로 변환)

		// 무작위로 밑면에서 위치를 선택
		float2 disk = float2(random.x, random.z) * baseRadius;

		// 각도에 따라 파티클이 생성되는 반경을 조정
		float angleFactor = 1.0f - (gAngle / 90.0f); // 각도가 커질수록 범위가 좁아짐
		float2 temp = disk;
		temp *= angleFactor;
		// 원뿔의 꼭짓점에서 파티클 생성
		posW = gEmitPosW + float3(temp.x, 0.0f, temp.y); // 꼭짓점에서 생성 위치 설정

		// 속도 벡터를 밑면 방향으로 설정하여 분사
		velocity = normalize(float3(disk.x, coneHeight, disk.y)) * gStartSpeed; // 방향을 밑면 쪽으로 설정
	}
}

struct Particle
{
	float3	InitialPosW		: POSITION;
	float3	InitialVelW		: VELOCITY;
	float2	SizeW			: SIZE;
	float	Age				: AGE;
	uint	Type			: TYPE;
};

float3x3 CreateRotationMatrixFromEuler(float3 eulerAngles)
{
	// 오일러 각도를 라디안 단위로 변환
	float pitch = radians(eulerAngles.x); // X축 회전 (Pitch)
	float yaw = radians(eulerAngles.y);   // Y축 회전 (Yaw)
	float roll = radians(eulerAngles.z);  // Z축 회전 (Roll)

	// 각 축에 대한 회전 행렬 계산
	float3x3 rotationX = {
		1, 0, 0,
		0, cos(pitch), -sin(pitch),
		0, sin(pitch), cos(pitch)
	};

	float3x3 rotationY = {
		cos(yaw), 0, sin(yaw),
		0, 1, 0,
		-sin(yaw), 0, cos(yaw)
	};

	float3x3 rotationZ = {
		cos(roll), -sin(roll), 0,
		sin(roll), cos(roll), 0,
		0, 0, 1
	};

	// 회전 순서를 ZYX (Roll -> Yaw -> Pitch)로 곱함
	return mul(rotationY, mul(rotationX, rotationZ));
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
[maxvertexcount(2)]
void StreamOutGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	// 현재 파티클의 나이 업데이트
	gin[0].Age += gTimeStep;

	// emitter 파티클인 경우
	if (gin[0].Type == PT_EMITTER)
	{
		if (gRestart)
		{
			gin[0].Age = 0.0f;        // emitter 나이를 초기화
		}

		// gIsLoop가 false일 때 Duration 동안만 파티클 생성
		if (gin[0].Age >= gDuration && !gIsLoop)
		{
			// Emitter 파티클을 스트림에 유지하면서 더 이상 새 파티클 생성 안 함
			ptStream.Append(gin[0]);
			// emitter 파티클 나이 초기화
			gin[0].Age = 0.0f;

			return;
		}

		// 새로운 파티클을 생성하여 스트림에 추가
		//if (gin[0].Age > 0.005f)
		if (gin[0].Age < gDuration || gIsLoop)
		{
			Particle p;

			// 3D 공간에서 무작위 방향 벡터 생성
			float3 vRandom = RandUnitVec3(0.0f);
			// x및 y성분을 축소하여 범위를 제한
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

			ProcessShape(vRandom, p.InitialPosW, p.InitialVelW);
			float3x3 rotationMatrix = CreateRotationMatrixFromEuler(gEmitDirW);
			p.InitialPosW = mul(rotationMatrix, p.InitialPosW - gEmitPosW) + gEmitPosW;
			p.InitialVelW = mul(rotationMatrix, p.InitialVelW);
			p.SizeW = gStartSize;	// 크기 설정
			p.Age = 0.0f;					// 나이 초기화
			p.Type = PT_SIMULATER;				// 파티클 유형 설정
			ptStream.Append(p);
		}

		// 방출기 입자 하나는 항상 유지한다.
		ptStream.Append(gin[0]);
	}
	else
	{
		// emitter 가 아닌 경우, 파티클의 나이가 Lifetime 보다 작거나 같은 경우에만 현재 스트림에 추가한다.
		if (gin[0].Age <= gStartLifetime)
			ptStream.Append(gin[0]);
	}
}


struct GeoOut
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 Tex   : TEXCOORD;
};

struct VertexOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint   Type  : TYPE;
};

// 파티클을 쿼드 형태로 변환하고 최종 출력한다.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
	inout TriangleStream<GeoOut> triStream)
{
	// 카메라의 시야 범위를 벗어나면 렌더링하지 않음
	float4 clipPos = mul(float4(gin[0].PosW, 1.0f), gViewProj);
	if (clipPos.x < -clipPos.w || clipPos.x > clipPos.w || clipPos.y < -clipPos.w || clipPos.y > clipPos.w || clipPos.z < 0.0f)
	{
		return;
	}

	// 쿼드 텍스처 좌표 정의
	const float2 gQuadTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};

	// do not draw emitter particles.
	if (gin[0].Type != PT_EMITTER)
	{
		//
		// Compute world matrix so that billboard faces the camera.
		//
		float3 look = normalize(gEyePosW.xyz - gin[0].PosW);	// 카메라를 향하는 방향 벡터
		float3 right = normalize(cross(float3(0, 1, 0), look));
		float3 up = cross(look, right);

		//
		// Computer triangle strip vertices (quad) in world space.
		//
		float halfWidth = 0.5f * gin[0].SizeW.x;
		float halfHeight = 0.5f * gin[0].SizeW.y;

		// 쿼드의 4개의 정점 위치를 계산
		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);

		//
		// Transform quad vertices to world space and output
		// them as a triangle strip.
		//
		GeoOut gout;
		[unroll]
		for (int i = 0; i < 4; ++i)
		{
			// 정점 위치를 변환하여 클립 공간으로 변환
			gout.PosH = mul(v[i], gViewProj);
			gout.PosH /= gout.PosH.w;
			gout.Tex = gQuadTexC[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}
	}
}

[maxvertexcount(3)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream< GeoOut > output
)
{
	for (uint i = 0; i < 3; i++)
	{
		GeoOut element;
	}
}

