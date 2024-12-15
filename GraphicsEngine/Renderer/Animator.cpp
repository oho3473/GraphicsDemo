#include "pch.h"	
#include <algorithm>
#include <utility>

#include "Animator.h"
#include "ModelData.h"
#include "Animation.h"
#include "Node.h"
#include "Mesh.h"
#include "ResourceManager.h"

void Animator::Initialize(std::weak_ptr<ResourceManager> manager)
{
	m_ResourceManager = manager;
}

void Animator::Update(std::vector<std::shared_ptr<RenderData>>& renderlist)
{
	m_socketList.clear();

	for (auto& data : renderlist)
	{
		if (data->isPlay)
		{
			UpdateWorld(data);
			UpdateMatrixPallete(data);
		}
	}
}

void Animator::UpdateWorld(std::weak_ptr<RenderData> ob)
{
	std::shared_ptr<RenderData> curOb = ob.lock();
	std::shared_ptr<ModelData> curModel;
	curModel = m_ResourceManager.lock()->Get<ModelData>(curOb->FBX).lock();	//하나의 fbx안에 여러개의 애니메이션

	//애니메이션 데이터가 유효한가?
	int curindex = curOb->curAni;	//fbx안에 여러 애니메이션이 존재 어떤 애니메이션을 쓸건지 index로 지정
	int preindex = curOb->preAni;	//fbx안에 여러 애니메이션이 존재 어떤 애니메이션을 쓸건지 index로 지정

	if (curModel == nullptr
		|| curModel->m_Animations.empty()
		|| curModel->m_Animations.size() <= curindex
		|| curModel->m_Animations.size() <= preindex
		|| curindex < 0
		|| preindex < 0)
	{
		return;
	}

	//현재 tick
	double curtick = curOb->duration * curModel->m_Animations[curindex]->m_TickFrame;
	double pretick = curOb->preDuration * curModel->m_Animations[preindex]->m_TickFrame;

	//이전 애니메이션과 현재 애니메이션이 같으면
	if (curindex == preindex)
	{
		std::vector<std::pair<float, DirectX::SimpleMath::Matrix>>::iterator cur;
		std::vector<std::pair<float, DirectX::SimpleMath::Matrix>>::iterator end;

		//그대로 연속 재생
		for (auto& ani : curModel->m_Animations[curindex]->m_Channels)
		{
			end = ani->totals.end()-1;
			//가지고 있는 애니메이션 정보 순회
			for (auto tick = ani->totals.begin(); tick != ani->totals.end(); tick++)
			{

				if (curtick >= end->first)
				{
					std::shared_ptr<Node> curAni = ani->node.lock();
					curAni->m_Local = end->second.Transpose();
					break;
				}

				//현재 애니메이션 위치 찾고 보간
				if (curtick < tick->first)
				{
					//tick이 curtick의 다음 애니메이션이다
					//그래서 현재 재생되고 있는 애니메이션은 tick - 1
					if (tick == ani->totals.begin())
					{
						cur = tick;
					}
					else
					{
						cur = tick - 1;
					}

					//현재 프레임이 최종 프레임보다 클 경우 다시 시작 점으로
					//end()가 끝이 아닌 마지막 + 1
					if (cur == end)
					{
						auto next = ani->totals.begin();
						float t = abs(curtick - cur->first) / abs(next->first - cur->first);

						std::shared_ptr<Node> curAni = ani->node.lock();
						curAni->m_Local = DirectX::SimpleMath::Matrix::Lerp(cur->second, next->second, t).Transpose();
						break;
					}

					//일반적인 보간
					auto next = tick;
					float t = abs(curtick - cur->first) / abs(next->first - cur->first);

					std::shared_ptr<Node> curAni = ani->node.lock();
					curAni->m_Local = DirectX::SimpleMath::Matrix::Lerp(cur->second, next->second, t).Transpose();
					break;
				}
			}
		}

	}
	else
	{

		float transitionDuration = curOb->transitionDuration; //  n초 동안 전환
		float transitionTime = curOb->duration; // 경과 시간
		float t = (transitionTime / transitionDuration);
		t = 1 - std::min<float>(t, 1.0f);
		//다른 애니메이션과 blending

		//이전 애니메이션 저장 공간
		std::unordered_map<std::wstring, DirectX::SimpleMath::Matrix> preAni;

		//이전 애니메이션의 키프레임 값 가져오기
		for (auto& ani : curModel->m_Animations[preindex]->m_Channels)
		{
			//가지고 있는 애니메이션 정보 순회
			for (auto tick = ani->totals.begin(); tick != ani->totals.end(); tick++)
			{
				if (pretick < tick->first)
				{
					preAni.insert(std::pair<std::wstring, DirectX::SimpleMath::Matrix >(ani->nodename, (tick - 1)->second));
				}
			}
		}

		//키프레임값을 못찾았다는 뜻 (틱이 마지막 틱보다 커서) 마지막 애니메이션을 넣어주자
		if (preAni.empty())
		{
			for (auto& ani : curModel->m_Animations[preindex]->m_Channels)
			{
				preAni.insert(std::pair<std::wstring, DirectX::SimpleMath::Matrix >(ani->nodename, (ani->totals.end() - 1)->second));
			}
		}


		//현재 애니메이션 - 시작까지 사이 보간
		for (auto& ani : curModel->m_Animations[curindex]->m_Channels)
		{
			auto start = ani->totals.begin();

			//현재꺼랑 공유하는 이전 애니메이션 노드 보간
			std::shared_ptr<Node> curAni = ani->node.lock();
			curAni->m_Local = DirectX::SimpleMath::Matrix::Lerp(start->second, preAni[ani->nodename], t).Transpose();
		}

	}

	CalcWorld(curOb->EntityID, curModel->m_Nodes);
}

void Animator::CalcWorld(uint32_t entityID, std::vector< std::shared_ptr<Node>>& Nodes)
{
	for (auto& node : Nodes)
	{
		if (!node->HasParents)
		{
			node->m_World = node->m_Local;
			node->m_WorldInverse = node->m_LocalInverse;
		}
		else
		{
			node->m_World = node->m_Parents.lock()->m_World * node->m_Local;
			node->m_WorldInverse = node->m_World.Invert();
		}

		//플레이어만 갱신해야댐
		{
			if (node->name == L"DEF-hand.R")
			{
				socket = node->m_World;
				m_socketList.push_back(std::pair(entityID, socket));
			}
		}

	}


}

void Animator::CalcWorld(uint32_t entityID, std::shared_ptr<Node> RootNode)
{
	if (!RootNode->HasParents)
	{
		RootNode->m_World = RootNode->m_Local;
		RootNode->m_WorldInverse = RootNode->m_LocalInverse;
	}
	else
	{
		RootNode->m_World = RootNode->m_Parents.lock()->m_World * RootNode->m_Local;
		RootNode->m_WorldInverse = RootNode->m_World.Invert();
	}

	//플레이어만 갱신해야댐
	{
		if (RootNode->name == L"DEF-hand.R")//
		{
			socket = RootNode->m_World;
			m_socketList.push_back(std::pair(entityID, socket));
		}
	}

	for (auto& node : RootNode->m_Childs)
	{
		CalcWorld(entityID, node);
	}
}

void Animator::UpdateMatrixPallete(std::shared_ptr<RenderData>& curData)
{
	std::shared_ptr<ResourceManager> resourcemanager = m_ResourceManager.lock();
	std::shared_ptr<ModelData> ob = resourcemanager->Get<ModelData>(curData->FBX).lock();
	std::wstring id = std::to_wstring(curData->EntityID);
	MatrixPallete& pallete = resourcemanager->Get<ConstantBuffer<MatrixPallete>>(id).lock()->m_struct;

	for (auto& mesh : ob->m_Meshes)
	{
		if (mesh->IsSkinned())
		{
			std::shared_ptr<SkinnedMesh> skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh);

			for (int i = 0; i < skinned->m_BoneData.size(); i++)
			{
				DirectX::SimpleMath::Matrix& nodeworld = skinned->m_BoneData[i]->node.lock()->m_World; //glocal
				DirectX::SimpleMath::Matrix& offset = skinned->m_BoneData[i]->offsetMatrix;
				pallete.offset[i] = (nodeworld * offset);
			}
		}
	}

	//Attachment(curData);
}

const DirectX::SimpleMath::Matrix Animator::Attachment(uint32_t entityID, std::wstring region)
{
	for (auto& socket : m_socketList)
	{
		if (socket.first == entityID)
		{
			return socket.second.Transpose();
		}
	}

	return DirectX::SimpleMath::Matrix::Identity;
}