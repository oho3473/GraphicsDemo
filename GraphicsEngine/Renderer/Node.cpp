#include "pch.h"
#include "Node.h"

Node::Node() : m_Local(DirectX::SimpleMath::Matrix::Identity), m_LocalInverse(DirectX::SimpleMath::Matrix::Identity),
m_World(DirectX::SimpleMath::Matrix::Identity),m_WorldInverse(DirectX::SimpleMath::Matrix::Identity),index(0),parentsindex(0)
{

}

Node::~Node()
{

}
