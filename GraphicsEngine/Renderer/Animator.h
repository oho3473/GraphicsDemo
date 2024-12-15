#pragma once
#include <string>
#include<vector>
#include <map>
#include <memory>

#include "RenderData.h"

class Node;
struct Key;
class ModelData;
class ResourceManager;

class Animator
{
public:
	Animator() = default;
	~Animator() = default;

	std::vector<std::shared_ptr<ModelData>> m_AnimationModel;

	void Initialize(std::weak_ptr<ResourceManager> manager);

	void Update(std::vector<std::shared_ptr<RenderData>>& renderlist);
	const DirectX::SimpleMath::Matrix Attachment(uint32_t entityID, std::wstring region);
	
private:


	void UpdateWorld(std::weak_ptr<RenderData> ob);
	void CalcWorld(uint32_t entityID, std::vector<std::shared_ptr<Node>>& Nodes);
	void CalcWorld(uint32_t entityID, std::shared_ptr<Node> RootNode);

	void UpdateMatrixPallete(std::shared_ptr<RenderData>& curData);

	std::weak_ptr<ResourceManager> m_ResourceManager;


	//calc nodes
	std::vector<std::shared_ptr<Node>> m_Nodes;


	DirectX::SimpleMath::Matrix socket;

	std::vector<std::pair<uint32_t, DirectX::SimpleMath::Matrix>> m_socketList;

};

