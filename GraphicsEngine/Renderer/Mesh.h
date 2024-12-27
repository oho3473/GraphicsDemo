#pragma once
#include "Resource.h"

#include <vector>
#include <string>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Vertex.h"

#include "Node.h"
#include "BufferData.h"

class Bone
{
public:
	Bone() : name(), offsetMatrix(), offsetMatrixInverse(),Boneindex(), node()
	{}

	std::wstring name;
	DirectX::SimpleMath::Matrix offsetMatrix;
	DirectX::SimpleMath::Matrix offsetMatrixInverse;
	//DirectX::SimpleMath::Matrix NodeMatrix;
	//float weight;
	int Boneindex;
	//int vertexid;
	//int ParentsId;
	std::vector<float> weights;
	std::vector<int> vertexids;
	std::weak_ptr<Node> node;
	
};

class Mesh 
{
public:
	Mesh();

	ID3D11Buffer* VB();
	ID3D11Buffer** GetAddressVB();
	UINT* VBSize();
	UINT* VBOffset();

	ID3D11Buffer* IB();
	ID3D11Buffer** GetAddressIB();
	UINT IBCount();

	std::weak_ptr<VertexBuffer> m_VB;
	std::weak_ptr<IndexBuffer> m_IB;
	D3D11_PRIMITIVE_TOPOLOGY m_primitive;
	int m_material;

	virtual bool IsSkinned() abstract;

	DirectX::XMFLOAT3 MinBounding;
	DirectX::XMFLOAT3 MaxBounding;
	DirectX::XMFLOAT3 Pivot;
};

class StaticMesh : public Mesh
{
public:
	StaticMesh();
	~StaticMesh();

	virtual bool IsSkinned() override final;

private:
};

class SkinnedMesh : public Mesh
{
public:
	SkinnedMesh();
	~SkinnedMesh();
	virtual bool IsSkinned() override final;

	std::weak_ptr<Node> m_node;
	std::vector<std::shared_ptr<Bone>> m_BoneData;
	MatrixPallete* Matrix_Pallete;

private:


};