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
	curModel = m_ResourceManager.lock()->Get<ModelData>(curOb->FBX).lock();	//�ϳ��� fbx�ȿ� �������� �ִϸ��̼�

	//�ִϸ��̼� �����Ͱ� ��ȿ�Ѱ�?
	int curindex = curOb->curAni;	//fbx�ȿ� ���� �ִϸ��̼��� ���� � �ִϸ��̼��� ������ index�� ����
	int preindex = curOb->preAni;	//fbx�ȿ� ���� �ִϸ��̼��� ���� � �ִϸ��̼��� ������ index�� ����

	if (curModel == nullptr
		|| curModel->m_Animations.empty()
		|| curModel->m_Animations.size() <= curindex
		|| curModel->m_Animations.size() <= preindex
		|| curindex < 0
		|| preindex < 0)
	{
		return;
	}

	//���� tick
	double curtick = curOb->duration * curModel->m_Animations[curindex]->m_TickFrame;
	double pretick = curOb->preDuration * curModel->m_Animations[preindex]->m_TickFrame;

	//���� �ִϸ��̼ǰ� ���� �ִϸ��̼��� ������
	if (curindex == preindex)
	{
		std::vector<std::pair<float, DirectX::SimpleMath::Matrix>>::iterator cur;
		std::vector<std::pair<float, DirectX::SimpleMath::Matrix>>::iterator end;

		//�״�� ���� ���
		for (auto& ani : curModel->m_Animations[curindex]->m_Channels)
		{
			end = ani->totals.end()-1;
			//������ �ִ� �ִϸ��̼� ���� ��ȸ
			for (auto tick = ani->totals.begin(); tick != ani->totals.end(); tick++)
			{

				if (curtick >= end->first)
				{
					std::shared_ptr<Node> curAni = ani->node.lock();
					curAni->m_Local = end->second.Transpose();
					break;
				}

				//���� �ִϸ��̼� ��ġ ã�� ����
				if (curtick < tick->first)
				{
					//tick�� curtick�� ���� �ִϸ��̼��̴�
					//�׷��� ���� ����ǰ� �ִ� �ִϸ��̼��� tick - 1
					if (tick == ani->totals.begin())
					{
						cur = tick;
					}
					else
					{
						cur = tick - 1;
					}

					//���� �������� ���� �����Ӻ��� Ŭ ��� �ٽ� ���� ������
					//end()�� ���� �ƴ� ������ + 1
					if (cur == end)
					{
						auto next = ani->totals.begin();
						float t = abs(curtick - cur->first) / abs(next->first - cur->first);

						std::shared_ptr<Node> curAni = ani->node.lock();
						curAni->m_Local = DirectX::SimpleMath::Matrix::Lerp(cur->second, next->second, t).Transpose();
						break;
					}

					//�Ϲ����� ����
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

		float transitionDuration = curOb->transitionDuration; //  n�� ���� ��ȯ
		float transitionTime = curOb->duration; // ��� �ð�
		float t = (transitionTime / transitionDuration);
		t = 1 - std::min<float>(t, 1.0f);
		//�ٸ� �ִϸ��̼ǰ� blending

		//���� �ִϸ��̼� ���� ����
		std::unordered_map<std::wstring, DirectX::SimpleMath::Matrix> preAni;

		//���� �ִϸ��̼��� Ű������ �� ��������
		for (auto& ani : curModel->m_Animations[preindex]->m_Channels)
		{
			//������ �ִ� �ִϸ��̼� ���� ��ȸ
			for (auto tick = ani->totals.begin(); tick != ani->totals.end(); tick++)
			{
				if (pretick < tick->first)
				{
					preAni.insert(std::pair<std::wstring, DirectX::SimpleMath::Matrix >(ani->nodename, (tick - 1)->second));
				}
			}
		}

		//Ű�����Ӱ��� ��ã�Ҵٴ� �� (ƽ�� ������ ƽ���� Ŀ��) ������ �ִϸ��̼��� �־�����
		if (preAni.empty())
		{
			for (auto& ani : curModel->m_Animations[preindex]->m_Channels)
			{
				preAni.insert(std::pair<std::wstring, DirectX::SimpleMath::Matrix >(ani->nodename, (ani->totals.end() - 1)->second));
			}
		}


		//���� �ִϸ��̼� - ���۱��� ���� ����
		for (auto& ani : curModel->m_Animations[curindex]->m_Channels)
		{
			auto start = ani->totals.begin();

			//���粨�� �����ϴ� ���� �ִϸ��̼� ��� ����
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

		//�÷��̾ �����ؾߴ�
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

	//�÷��̾ �����ؾߴ�
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