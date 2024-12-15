#pragma once
#include "pch.h"
#include <map>

#pragma region assimp
#include "scene.h"
#include "Importer.hpp"
#include "postprocess.h"
#pragma endregion assimp

#include "VertexData.h"



class Device;
class ResourceManager;
class ModelData;
class Node;
class Mesh;

enum class Filter
{
	SKINNING = 0,
	STATIC = 1,

	END
};

class ModelLoader
{
public:
	ModelLoader() = default;
	~ModelLoader() = default;

	void Initialize(const std::shared_ptr<ResourceManager>& manager, const std::shared_ptr<Device>& device);
	bool LoadModel(std::string filename, Filter filter, int UID);
	void ProcessSceneData(std::string name, const aiScene* scene, Filter filter, int UID);

private:

	void SaveBoneDataTexture(std::shared_ptr<ModelData> newData);

private:
	std::vector<const aiScene*> m_SceneList;

	void ProcessMesh(std::shared_ptr<ModelData> Model, aiMesh* mesh, unsigned int index, Filter filter);
	void ProcessMaterials(std::shared_ptr<ModelData> Model, aiMaterial* material);
	void ProcessBoneNodeMapping(std::shared_ptr<ModelData> Model);
	void ProcessAnimation(std::shared_ptr<ModelData> Model, aiAnimation* animation);

	void ProcessVertexBuffer(std::vector<SkinningVertex>& buffer, aiMesh* curMesh, unsigned int index);
	void ProcessVertexBuffer(std::vector<BaseVertex>& buffer, aiMesh* curMesh, unsigned int index);

	void ProcessIndexBuffer(std::vector<UINT>& buffer, aiFace* curFace);
	void ProcessNode(std::shared_ptr<Node> parents, std::shared_ptr<Node> ob_node, aiNode* node, std::vector<std::shared_ptr<Mesh>>& meshes, std::vector<std::shared_ptr<Node>>&nodes ,int index=0);

	void ProcessBoneMapping(std::vector<SkinningVertex>& buffer, aiMesh* curAiMesh, std::shared_ptr<Mesh> curMesh);
	std::shared_ptr<Node>FindNode(std::wstring nodename, std::shared_ptr<Node> RootNode);


	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;

	std::map<std::wstring, int> BoneMapping;

private:

	std::map<Filter, std::vector<std::string>> m_ResourceDirectory;

};
