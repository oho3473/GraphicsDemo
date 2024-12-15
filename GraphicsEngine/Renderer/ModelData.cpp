#include "pch.h"
#include "ModelData.h"

ModelData::ModelData() : m_name(L"need name"), m_RootNode(), RS()
{

}

ModelData::ModelData(std::shared_ptr<ModelData> other)
{
	if (other != nullptr)
	{
		this->m_RootNode = other->m_RootNode;
		this->m_Meshes = other->m_Meshes;
		this->m_Materials = other->m_Materials;
		this->m_Animations = other->m_Animations;
		//this->m_pass = other->m_pass;
	}
}

ModelData::~ModelData()
{

}


void ModelData::Release()
{
	for (auto& mesh : m_Meshes)
	{
		mesh.reset();
	}


	for (auto& material : m_Materials)
	{
		material.reset();
	}


	for (auto& ani : m_Animations)
	{
		ani.reset();
	}
}
