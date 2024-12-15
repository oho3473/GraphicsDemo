#pragma once
#include "RenderData.h"
#include <queue>
#include <map>
#include <string>
#include <memory>

///2024.10.18
///���¿�
///Decalobject�� ������ �Ŵ��� Ŭ����

class ShaderResourceView;
class ResourceManager;

class DecalManager
{
public:
	DecalManager();

	void Initialize(std::weak_ptr<ResourceManager> manager);
	void AddTask(decal::Info info);
	void ClearDecals();



	std::map<std::string, std::vector<decal::Info>>& GetDecals();
	std::weak_ptr<ShaderResourceView> GetNormalDecal(std::string name);
private:

	std::map<std::string, std::vector<decal::Info>> m_decals;
	std::map<std::string, std::weak_ptr<ShaderResourceView>> m_normaldecals;
};

