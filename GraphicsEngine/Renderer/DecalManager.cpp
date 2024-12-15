#include "pch.h"
#include "DecalManager.h"
#include "ResourceManager.h"
#include "ShaderResourceView.h"

DecalManager::DecalManager()
{
}

void DecalManager::Initialize(std::weak_ptr<ResourceManager> manager)
{
	m_normaldecals.insert({ "Decal(1).png",manager.lock()->Get<ShaderResourceView>(L"Decal(1)_N.png") });
	m_normaldecals.insert({ "Decal(2).png",manager.lock()->Get<ShaderResourceView>(L"Decal(2)_N.png") });
	m_normaldecals.insert({ "Decal(3).png",manager.lock()->Get<ShaderResourceView>(L"Decal(3)_N.png") });
	m_normaldecals.insert({ "Decal(4).png",manager.lock()->Get<ShaderResourceView>(L"Decal(4)_N.png") });

}

void DecalManager::AddTask(decal::Info info)
{
	if (m_decals.find(info.TexturePath) != m_decals.end())
	{
		m_decals[info.TexturePath].push_back(info);
		return;
	}
	else
	{
		std::vector < decal::Info > temp;
		temp.push_back(info);
		m_decals.insert(std::pair<std::string, std::vector<decal::Info>>(info.TexturePath, temp));
	}
}

void DecalManager::ClearDecals()
{

	if (!m_decals.empty())
	{
		m_decals.clear();
	}
}

std::map<std::string, std::vector<decal::Info>>& DecalManager::GetDecals()
{
	return  m_decals;
}

std::weak_ptr<ShaderResourceView> DecalManager::GetNormalDecal(std::string name)
{
	if (m_normaldecals.find(name) != m_normaldecals.end())
	{
		return m_normaldecals[name];
	}

	return {};
}
