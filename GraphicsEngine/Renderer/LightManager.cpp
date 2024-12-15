#include "pch.h"
#include "LightManager.h"
#include "ResourceManager.h"
#include "Desc.h"

#include <iostream>
#include <filesystem>

LightManager::~LightManager()
{
	m_LightMap.clear();
}

void LightManager::Initialize(std::weak_ptr<ResourceManager> manager)
{
	m_ResourceManager = manager;
	std::string path;
	path = "..\\Data\\LightMap";

	if (!std::filesystem::exists(path))
	{
		//디렉토리 없으면 만들기
		std::filesystem::create_directories(path);
	}
	else
	{
		std::vector<std::string> filelist;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string filename = std::filesystem::path(entry).filename().string();
			filelist.push_back(filename);
		}

		for (auto& file : filelist)
		{
			std::wstring wfilename;
			wfilename.assign(file.begin(), file.end());
		}

		m_srvArray = m_ResourceManager.lock()->Create<ShaderResourceView>(L"LightMapArray", filelist, ShaderResourceView::Directory::LightMap);
	}

}

void LightManager::EraseData(uint32_t EntityID, LightType type)
{
	int index = static_cast<int>(type);
	std::unordered_map<uint32_t, LightData>& curMap = m_LightList[index];

	if (curMap.find(EntityID) != curMap.end())
	{
		curMap.erase(EntityID);
	}
}

void LightManager::Update(std::unordered_map<uint32_t, LightData>& usinglight)
{
	int dirIndex = 0;
	int pointIndex = 0;
	int spotIndex = 0;

	std::vector<LightData> dir;
	std::vector<LightData> point;
	std::vector<LightData> spot;

	for (auto& curData : usinglight)
	{
		LightData curLight = curData.second;
		LightType type = static_cast<LightType>(curLight.type);

		switch (type)
		{
			case LightType::Direction:
			{
				dirIndex++;
				dir.push_back(curLight);
			}
			break;
			case LightType::Spot:
			{
				spotIndex++;
				spot.push_back(curLight);
			}
			break;
			case LightType::Point:
			{
				pointIndex++;
				point.push_back(curLight);
			}
			break;
			case LightType::End:
				break;
		}
	}

	m_BufferStruct.DirIndex = dirIndex;
	m_BufferStruct.PointIndex = pointIndex;
	m_BufferStruct.SpotIndex = spotIndex;

	for (int i = 0; i < dirIndex; i++)
	{
		m_BufferStruct.array[i] = dir[i];
	}

	for (int i = 0; i < spotIndex; i++)
	{
		m_BufferStruct.array[dirIndex + i] = spot[i];
	}

	for (int i = 0; i < pointIndex; i++)
	{
		m_BufferStruct.array[dirIndex + spotIndex + i] = point[i];
	}

	std::shared_ptr<ConstantBuffer<LightArray>> cbuffer = m_ResourceManager.lock()->Get<ConstantBuffer<LightArray>>(L"LightArray").lock();
	cbuffer->Update(m_BufferStruct);
}

std::weak_ptr<ShaderResourceView> LightManager::GetLightMap(int index)
{

	if (m_LightMap.size() > index)
	{
		return m_LightMap[index];
	}
	else
	{
		//std::wstring text = L"LightMap Index not exist";
		//MessageBox(0, text.c_str(), 0, 0);
	}

	return {};
}
std::weak_ptr<ShaderResourceView> LightManager::GetLightMaps()

{
	return m_srvArray;
}

