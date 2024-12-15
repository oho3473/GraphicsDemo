#include "pch.h"
#include "Node.h"

Node::Node() : m_Local(DirectX::SimpleMath::Matrix::Identity), m_LocalInverse(DirectX::SimpleMath::Matrix::Identity),
m_World(DirectX::SimpleMath::Matrix::Identity),m_WorldInverse(DirectX::SimpleMath::Matrix::Identity)
{

}

Node::~Node()
{

}
