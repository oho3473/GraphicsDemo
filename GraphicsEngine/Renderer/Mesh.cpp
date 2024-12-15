#include "pch.h"

#include "Mesh.h"

Mesh::Mesh() : m_VB(), m_IB(), m_primitive()
{

}

ID3D11Buffer* Mesh::VB()
{
	return m_VB.lock()->Get();
}

ID3D11Buffer* Mesh::IB()
{
	return m_IB.lock()->Get();
}

ID3D11Buffer** Mesh::GetAddressVB()
{
	return m_VB.lock()->GetAddress();
}

UINT* Mesh::VBSize()
{
	return m_VB.lock()->Size();
}

UINT* Mesh::VBOffset()
{
	return m_VB.lock()->Offset();
}

ID3D11Buffer** Mesh::GetAddressIB()
{
	return m_IB.lock()->GetAddress();
}

UINT Mesh::IBCount()
{
	return m_IB.lock()->Count();
}

StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{
}

bool StaticMesh::IsSkinned()
{
	return false;
}

SkinnedMesh::SkinnedMesh() : Mesh(), m_node(), Matrix_Pallete(new MatrixPallete())
{

}

SkinnedMesh::~SkinnedMesh()
{
	delete Matrix_Pallete;
}

bool SkinnedMesh::IsSkinned()
{
	return true;
}
