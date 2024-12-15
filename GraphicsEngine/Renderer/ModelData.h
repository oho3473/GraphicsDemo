#pragma once
#include "Resource.h"
#include "RenderData.h"

#include <string>

#include<memory>

class Mesh;
class Material;
class Animation;
class Node;
class RenderState;




class ModelData :
	public Resource, public std::enable_shared_from_this<ModelData>
{
public:
	ModelData();
	~ModelData();

	ModelData(std::shared_ptr<ModelData> other);


	virtual void Release() override;

	std::wstring m_name;
	std::shared_ptr<Node> m_RootNode;	//node 전체를 담고 있는 컨테이너
	std::vector<std::shared_ptr<Node>>m_Nodes;


	std::vector<std::shared_ptr<Mesh>> m_Meshes; // VB + IB + BONE + PRIMITIVE
	std::vector<std::shared_ptr<Material>> m_Materials; //SRV
	std::vector<std::shared_ptr<Animation>> m_Animations; //애니메이션 전체를 담고있는 컨테이너

	std::weak_ptr<RenderState>  RS;

	
	std::vector<DirectX::SimpleMath::Vector3> vertices;

	int UID;
	DirectX::SimpleMath::Vector3 Pivot;
};

