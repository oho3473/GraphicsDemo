#pragma once
#include <memory>
#include "SimpleMath.h"
#include <string>
#include <vector>

class Mesh;
class Bone;

//사실상 얘가 본이네... 
class Node
{
public:
	Node();
	~Node();

	std::wstring name;

	DirectX::SimpleMath::Matrix m_Local;
	DirectX::SimpleMath::Matrix m_LocalInverse;
	DirectX::SimpleMath::Matrix m_World;
	DirectX::SimpleMath::Matrix m_WorldInverse;

	unsigned int index;
	unsigned int parentsindex;

	std::vector<std::weak_ptr<Mesh>> m_Meshes;

	std::weak_ptr<Node> m_Parents;
	bool HasParents = false;

	std::vector<std::shared_ptr<Node>> m_Childs;
};

