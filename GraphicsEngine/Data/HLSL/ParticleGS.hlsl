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

// ���� �ð��� ���� ������ offset�� ������� ���� ���͸� ���ø����ش�.
float3 RandUnitVec3(float offset)
{
	// ���� �ð� ���ϱ� �������� ������ �ؽ�ó ������ ����
	// �ؽ�ó ��ǥ�� ����Ѵ�.
	float u = (gGameTime + offset);

	// ���� ���е��� ������ [-1, 1] �̴�.
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	// ���� ���� �����Ѵ�.
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

		// ��ƼŬ�� ��ġ�� ���� �߽�(gEmitPosW)���� �����ϵ��� ����
		posW = gEmitPosW + randomDir * gRadius; // ���� ǥ�鿡�� ����

		// �ӵ� ���͸� randomDir�� ������� ����
		velocity = randomDir * gStartSpeed; // ���� ǥ�鿡�� �ٱ������� ����
	}
	else if (gParticleShape == SHAPE_BOX)
	{
		posW = gEmitPosW.xyz;
		velocity = gStartSpeed * random;
	}
	else if (gParticleShape == SHAPE_CONE)
	{
		// ������ ���̿� �ظ� �������� ����
		float coneHeight = gRadius; // ������ ����
		float baseRadius = coneHeight * tan(radians(gAngle)); // �ظ��� ������ ��� (radians �Լ��� ������ ���� ������ ��ȯ)

		// �������� �ظ鿡�� ��ġ�� ����
		float2 disk = float2(random.x, random.z) * baseRadius;

		// ������ ���� ��ƼŬ�� �����Ǵ� �ݰ��� ����
		float angleFactor = 1.0f - (gAngle / 90.0f); // ������ Ŀ������ ������ ������
		float2 temp = disk;
		temp *= angleFactor;
		// ������ ���������� ��ƼŬ ����
		posW = gEmitPosW + float3(temp.x, 0.0f, temp.y); // ���������� ���� ��ġ ����

		// �ӵ� ���͸� �ظ� �������� �����Ͽ� �л�
		velocity = normalize(float3(disk.x, coneHeight, disk.y)) * gStartSpeed; // ������ �ظ� ������ ����
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
	// ���Ϸ� ������ ���� ������ ��ȯ
	float pitch = radians(eulerAngles.x); // X�� ȸ�� (Pitch)
	float yaw = radians(eulerAngles.y);   // Y�� ȸ�� (Yaw)
	float roll = radians(eulerAngles.z);  // Z�� ȸ�� (Roll)

	// �� �࿡ ���� ȸ�� ��� ���
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

	// ȸ�� ������ ZYX (Roll -> Yaw -> Pitch)�� ����
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
	// ���� ��ƼŬ�� ���� ������Ʈ
	gin[0].Age += gTimeStep;

	// emitter ��ƼŬ�� ���
	if (gin[0].Type == PT_EMITTER)
	{
		if (gRestart)
		{
			gin[0].Age = 0.0f;        // emitter ���̸� �ʱ�ȭ
		}

		// gIsLoop�� false�� �� Duration ���ȸ� ��ƼŬ ����
		if (gin[0].Age >= gDuration && !gIsLoop)
		{
			// Emitter ��ƼŬ�� ��Ʈ���� �����ϸ鼭 �� �̻� �� ��ƼŬ ���� �� ��
			ptStream.Append(gin[0]);
			// emitter ��ƼŬ ���� �ʱ�ȭ
			gin[0].Age = 0.0f;

			return;
		}

		// ���ο� ��ƼŬ�� �����Ͽ� ��Ʈ���� �߰�
		//if (gin[0].Age > 0.005f)
		if (gin[0].Age < gDuration || gIsLoop)
		{
			Particle p;

			// 3D �������� ������ ���� ���� ����
			float3 vRandom = RandUnitVec3(0.0f);
			// x�� y������ ����Ͽ� ������ ����
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

			ProcessShape(vRandom, p.InitialPosW, p.InitialVelW);
			float3x3 rotationMatrix = CreateRotationMatrixFromEuler(gEmitDirW);
			p.InitialPosW = mul(rotationMatrix, p.InitialPosW - gEmitPosW) + gEmitPosW;
			p.InitialVelW = mul(rotationMatrix, p.InitialVelW);
			p.SizeW = gStartSize;	// ũ�� ����
			p.Age = 0.0f;					// ���� �ʱ�ȭ
			p.Type = PT_SIMULATER;				// ��ƼŬ ���� ����
			ptStream.Append(p);
		}

		// ����� ���� �ϳ��� �׻� �����Ѵ�.
		ptStream.Append(gin[0]);
	}
	else
	{
		// emitter �� �ƴ� ���, ��ƼŬ�� ���̰� Lifetime ���� �۰ų� ���� ��쿡�� ���� ��Ʈ���� �߰��Ѵ�.
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

// ��ƼŬ�� ���� ���·� ��ȯ�ϰ� ���� ����Ѵ�.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
	inout TriangleStream<GeoOut> triStream)
{
	// ī�޶��� �þ� ������ ����� ���������� ����
	float4 clipPos = mul(float4(gin[0].PosW, 1.0f), gViewProj);
	if (clipPos.x < -clipPos.w || clipPos.x > clipPos.w || clipPos.y < -clipPos.w || clipPos.y > clipPos.w || clipPos.z < 0.0f)
	{
		return;
	}

	// ���� �ؽ�ó ��ǥ ����
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
		float3 look = normalize(gEyePosW.xyz - gin[0].PosW);	// ī�޶� ���ϴ� ���� ����
		float3 right = normalize(cross(float3(0, 1, 0), look));
		float3 up = cross(look, right);

		//
		// Computer triangle strip vertices (quad) in world space.
		//
		float halfWidth = 0.5f * gin[0].SizeW.x;
		float halfHeight = 0.5f * gin[0].SizeW.y;

		// ������ 4���� ���� ��ġ�� ���
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
			// ���� ��ġ�� ��ȯ�Ͽ� Ŭ�� �������� ��ȯ
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

