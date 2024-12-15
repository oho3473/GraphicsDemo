#pragma once
#pragma once
#include "VertexData.h"


namespace MyColor
{
	static DirectX::XMFLOAT4 Red = { 1.0f,0.f,0.f,1.0f };
	static DirectX::XMFLOAT4 Green = { 0.0f,1.f,0.f,1.0f };
	static DirectX::XMFLOAT4 Blue = { 0.0f,0.f,1.f,1.0f };
	static DirectX::XMFLOAT4 Gray = { 0.5f,0.5f,0.5f,1.0f };
	static DirectX::XMFLOAT4 Black = { 0.0f,0.0f,0.0f,0.0f };
	static DirectX::XMFLOAT4 Zero = { 0.0f,0.0f,0.0f,0.0f };
}




namespace Axis
{
	namespace Vertex
	{
		static BaseVertex Buffer[] = {
				{DirectX::XMFLOAT4(0.f, 0.f, 0.f,1.0f), { 1.f, 0.f, 0.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},
				{DirectX::XMFLOAT4(10.f, 0.f, 0.f,1.0f), { 1.f, 0.f, 0.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},

				{DirectX::XMFLOAT4(0.f, 0.f, 0.f,1.0f), { 0.f, 1.f, 0.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},
				{DirectX::XMFLOAT4(0.f, 10.f, 0.f,1.0f), { 0.f, 1.f, 0.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},

				{DirectX::XMFLOAT4(0.f, 0.f, 0.f,1.0f), { 0.f, 0.f, 1.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},
				{DirectX::XMFLOAT4(0.f, 0.f, 10.f,1.0f), { 0.f, 0.f, 1.f, 1.f },{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f, 0.f,0.0f},{0.f, 0.f}},
		};

		static UINT count = 6;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(BaseVertex) * Axis::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data =
		{
			Axis::Vertex::Buffer
		};
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
			0,1,
			2,3,
			4,5
		};

		static UINT count = 6;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * Axis::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { Axis::Index::Buffer };
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
}

namespace Box
{
	namespace Vertex
	{

		static BaseVertex Buffer[] = {
				{DirectX::XMFLOAT4(-1.f,-1.f, -1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},
				{DirectX::XMFLOAT4(-1.f, 1.f, -1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},

				{DirectX::XMFLOAT4(1.f, 1.f, -1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},
				{DirectX::XMFLOAT4(1.f,-1.f, -1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},

				{DirectX::XMFLOAT4(-1.f,-1.f, 1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},
				{DirectX::XMFLOAT4(-1.f, 1.f, 1.f,1.0f), {0.5f, 0.5f, 0.5f, 1.f }},

				{DirectX::XMFLOAT4(1.f, 1.f, 1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},
				{DirectX::XMFLOAT4(1.f,-1.f, 1.f,1.0f), { 0.5f, 0.5f, 0.5f, 1.f }},
		};

		static UINT count = 8;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(BaseVertex) * Box::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data =
		{
			Box::Vertex::Buffer
		};
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
		0,1,2,
		0,2,3,

		4,6,5,
		4,7,6,

		1,4,5,
		0,4,1,

		3,2,6,
		3,6,7,

		1,5,6,
		1,6,2,

		3,4,0,
		7,4,3,
		};

		static UINT count = 36;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * Box::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data =
		{
			Box::Index::Buffer
		};
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

namespace TextureBox
{
	namespace Vertex
	{

		static float scale = 1;
		static BaseVertex Buffer[] = {
			//face 단위로 스플릿

			//윗면 4개 + 아래도 4개 해서 육면체니까 24개
			//인덱스는 face당 3개씩 12개 * 3개 
			//8개의 점을 3개의 노말 값으로 분산
			//앞면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) },//1 //0
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) },//2 //1
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) },//3 //2
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) },//4 //3

			// 뒷면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) }, //5 //4
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) }, //6 //5
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) }, //7 //6
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) }, //8 //7

			//// 윗면
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) }, //2 //8
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) }, //3 //9
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) }, //6 //10
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) }, //7 //11

			// 아랫면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) }, //1 //12
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) }, //4 //13
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) }, //5 //14
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) }, //8 //15

			//왼쪽면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) }, //1 //16
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) }, //2 //17
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) }, //5 //18
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) }, //6 //19

			// 오른쪽면
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,0.0f) }, //3 //20
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,0.0f) }, //4 //21
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(1.0f,1.0f) }, //7 //22
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), MyColor::Black, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0},{0,0,0,0},DirectX::XMFLOAT2(0.0f,1.0f) }  //8 //234		
		};

		static UINT count = 24; // 8 * 3;


		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(BaseVertex) * TextureBox::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { TextureBox::Vertex::Buffer };
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
			// front face
			1,2,0,
			2,3,0,

			// back face
			6,5,7,
			5,4,7,

			// left face
			19,17,18,
			17,16,18,

			// right face
			20,22,21,
			22,23,21,

			// top face
			10,11,8,
			11,9,8,

			// bottom face
			12,13,14,
			13,15,14
		};

		static UINT count = 36;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * TextureBox::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { TextureBox::Index::Buffer };
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}


namespace Quad
{
	namespace Vertex
	{
		//물론 dsv가 있을 경우
		//z값을 1로하는 이유는 foward를 하는 경우에는 quad(texture)가 깊이값에 의해 덮어 그려버릴수도있다

		static float depth = 0.f;

		static QuadVertex Buffer[] = {

		{ DirectX::XMFLOAT4(-1.0f , -1.0f , depth, 1.f),DirectX::XMFLOAT2(0.0f,1.0f) },// 왼쪽 아래
		{ DirectX::XMFLOAT4(-1.0f ,  1.0f , depth, 1.f),DirectX::XMFLOAT2(0.0f,0.0f) },// 왼쪽 위
		{ DirectX::XMFLOAT4(1.0f  ,  1.0f , depth, 1.f),DirectX::XMFLOAT2(1.0f,0.0f) },// 오른쪽 위
		{ DirectX::XMFLOAT4(1.0f  , -1.0f , depth, 1.f),DirectX::XMFLOAT2(1.0f,1.0f) },// 오른쪽 아래


		};

		static UINT count = 4;


		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(QuadVertex) * Quad::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { Quad::Vertex::Buffer };
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
			// front face
			0,1,2,
			0,2,3,
		};

		static UINT count = 6;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * Quad::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { Quad::Index::Buffer };
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

namespace Quad_Deth1
{
	namespace Vertex
	{
		//물론 dsv가 있을 경우
		//z값을 1로하는 이유는 foward를 하는 경우에는 quad(texture)가 깊이값에 의해 덮어 그려버릴수도있다

		static float depth = 0.99f;

		static QuadVertex Buffer[] = {

		{ DirectX::XMFLOAT4(-1.0f , -1.0f , depth, 1.f),DirectX::XMFLOAT2(0.0f,1.0f) },// 왼쪽 아래
		{ DirectX::XMFLOAT4(-1.0f ,  1.0f , depth, 1.f),DirectX::XMFLOAT2(0.0f,0.0f) },// 왼쪽 위
		{ DirectX::XMFLOAT4(1.0f  ,  1.0f , depth, 1.f),DirectX::XMFLOAT2(1.0f,0.0f) },// 오른쪽 위
		{ DirectX::XMFLOAT4(1.0f  , -1.0f , depth, 1.f),DirectX::XMFLOAT2(1.0f,1.0f) },// 오른쪽 아래


		};

		static UINT count = 4;


		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(QuadVertex) * Quad::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { Quad::Vertex::Buffer };
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
			// front face
			0,1,2,
			0,2,3,
		};

		static UINT count = 6;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * Quad::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { Quad::Index::Buffer };
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}



namespace DecalVolume
{
	namespace Vertex
	{
		static float scale = 0.5f;
		static DecalVertex Buffer[] = {
			//face 단위로 스플릿

			//윗면 4개 + 아래도 4개 해서 육면체니까 24개
			//인덱스는 face당 3개씩 12개 * 3개 
			//8개의 점을 3개의 노말 값으로 분산
			//앞면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f),DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,DirectX::XMFLOAT4(0.0f,1.0f,0,0) },//1 //0
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f),DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,DirectX::XMFLOAT4(0.0f,0.0f,0,0) },//2 //1
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f),DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,DirectX::XMFLOAT4(1.0f,0.0f,0,0) },//3 //2
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f),DirectX::XMFLOAT4(0.0f, 0.0f,-1.0f,0) ,DirectX::XMFLOAT4(1.0f,1.0f,0,0) },//4 //3

			// 뒷면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0}}, //5 //4
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0}}, //6 //5
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0}}, //7 //6
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f,0) ,{0,0,0,0}}, //8 //7

			//// 윗면
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0), {0,0,0,0}}, //2 //8
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0), {0,0,0,0}}, //3 //9
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0), {0,0,0,0}}, //6 //10
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f,0), {0,0,0,0}}, //7 //11

			// 아랫면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0}}, //1 //12
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0}}, //4 //13
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0}}, //5 //14
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(0.0f, -1.0f,0.0f,0) ,{0,0,0,0}}, //8 //15

			//왼쪽면
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0} }, //1 //16
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0} }, //2 //17
			{ DirectX::XMFLOAT4(-1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0} }, //5 //18
			{ DirectX::XMFLOAT4(-1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(-1.0f, 0.0f,0.0f,0) ,{0,0,0,0} }, //6 //19

			// 오른쪽면
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0}, }, //3 //20
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, -1.0f * scale,1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0}, }, //4 //21
			{ DirectX::XMFLOAT4(+1.0f * scale, +1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0}, }, //7 //22
			{ DirectX::XMFLOAT4(+1.0f * scale, -1.0f * scale, +1.0f * scale,1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f,0), {0,0,0,0}, }  //8 //234		
		};

		static UINT count = 24; // 8 * 3;


		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(DecalVertex) * DecalVolume::Vertex::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { DecalVolume::Vertex::Buffer };
	}

	namespace Index
	{
		static UINT Buffer[] =
		{
			// front face
			1,2,0,
			2,3,0,

			// back face
			6,5,7,
			5,4,7,

			// left face
			19,17,18,
			17,16,18,

			// right face
			20,22,21,
			22,23,21,

			// top face
			10,11,8,
			11,9,8,

			// bottom face
			12,13,14,
			13,15,14
		};

		static UINT count = 36;

		static D3D11_BUFFER_DESC Desc =
		{
			sizeof(UINT) * DecalVolume::Index::count,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_INDEX_BUFFER,
			0,
			0,
			0
		};

		static D3D11_SUBRESOURCE_DATA Data = { DecalVolume::Index::Buffer };
	}

	static D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}