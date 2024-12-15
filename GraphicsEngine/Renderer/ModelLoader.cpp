#include "pch.h"

#include <filesystem>
#include <iostream>
#include <memory>

#include "ModelLoader.h"

#include "DirectXTex.h"
#include "DirectXHelpers.h"
#include "ResourceManager.h"

#include "ModelData.h"
#include "Material.h"
#include "Node.h"
#include "Mesh.h"
#include "Animation.h"

#include "ShaderResourceView.h"
#include "ConstantBuffer.h"
#include "Texture2D.h"

#include "Desc.h"
#include "BufferData.h"

#include "pbrmaterial.h"


#include "Util.h"

void ModelLoader::Initialize(const std::shared_ptr<ResourceManager>& manager, const std::shared_ptr<Device>& device)
{
	m_ResourceManager = manager;
	m_Device = device;

	std::string path;

	path = "..\\Data\\FBX\\";


	if (!std::filesystem::exists(path))
	{
		//디렉토리 없으면 만들기
		std::filesystem::create_directories(path);
		std::filesystem::create_directories(path + "SKINNING");
		std::filesystem::create_directories(path + "STATIC");
	}
	else
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::vector<std::string> filelist;
			std::string filename = std::filesystem::path(entry).filename().string();
			Filter curFilter;
			if (filename == "SKINNING")
			{
				curFilter = Filter::SKINNING;
			}
			else
			{
				curFilter = Filter::STATIC;
			}

			for (const auto& entry2 : std::filesystem::directory_iterator(entry))
			{
				std::string filename = std::filesystem::path(entry2).filename().string();

				filelist.push_back(filename);
			}

			m_ResourceDirectory.insert({ curFilter,filelist });
		}

		//여기서 리소스 많이 들어가면 dt ㅈㄴ 늘어나서 애니메이션이 터짐 - dt값이 튀어서 - 늘어날때마다 매번 함수 넣어줄 수는 없자나
		//멀티 스레드면 참 좋을듯

		static int UID = 1;	//0은 아무것도 없는것
		for (auto& dir : m_ResourceDirectory)
		{
			for (auto& file : dir.second)
			{
				LoadModel(file, dir.first, UID);
				UID++;
			}
		}
	}
}

bool ModelLoader::LoadModel(std::string filename, Filter filter, int UID)
{
	//std::filesystem::path path = ToWString(std::string(filename));


	std::string filePath = "..\\Data\\FBX\\";
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 노드 생성안함

	//플래그를 모델을 어떻게 읽을지 정해야하니까 매번 어케 읽을지 셋팅해주는게 좋아임
	unsigned int importFlags{};


	switch (filter)
	{
		case Filter::STATIC:
			filePath = filePath + "STATIC\\";

			importFlags = aiProcess_Triangulate // 삼각형으로 변환
				| aiProcess_GenNormals 	// 노말 생성/
				| aiProcess_GenUVCoords 		// UV 생성
				| aiProcess_CalcTangentSpace   // 탄젠트 생성			
				| aiProcess_GenBoundingBoxes  // 바운딩 박스 생성
				| aiProcess_PreTransformVertices  // 노드의 변환행렬을 적용한 버텍스 생성 /주의 이 단계에서는 애니메이션이 제거됩니다.
				| aiProcess_GlobalScale 	//단위를 미터로 설정할 수 있습니다.
				| aiProcess_ConvertToLeftHanded;	// 왼손 좌표계로 변환

			break;
		case Filter::SKINNING:
			filePath = filePath + "SKINNING\\";

			importFlags = aiProcess_Triangulate  // 삼각형으로 변환
				| aiProcess_GenNormals 	// 노말 생성/
				| aiProcess_GenUVCoords 		// UV 생성
				| aiProcess_CalcTangentSpace   // 탄젠트 생성			
				| aiProcess_GenBoundingBoxes  // 바운딩 박스 생성
				| aiProcess_GlobalScale 	//단위를 미터로 설정할 수 있습니다.
				| aiProcess_ConvertToLeftHanded;	// 왼손 좌표계로 변환
			/*
			aiProcess_LimitBoneWeights | // 본에 영향을 받는 정점의 최대 개수를 4개로 제한 - 일부 메쉬는 이거에 영향을 받아 뒤틀린다.. 이거 처리가 필요하다
			aiProcess_FlipUVs|
			aiProcess_FlipWindingOrder|
			*/
			break;
		case Filter::END:
			break;
		default:
			break;
	}

	filePath += filename;

	const aiScene* scene = importer.ReadFile(filePath, importFlags);
	if (!scene)
	{
		std::wstring wfilename;
		wfilename.assign(filename.begin(), filename.end());

		wfilename = L"Error loading files : " + wfilename;
		LPCWSTR name = wfilename.c_str();
		MessageBox(0, name, 0, 0);
		return false;
	}

	ProcessSceneData(filename, scene, filter, UID);

	importer.FreeScene();
	return true;

}

//데이터 가공이 필요하다
void ModelLoader::ProcessSceneData(std::string name, const aiScene* scene, Filter filter, int UID)
{
	std::shared_ptr<ModelData> newData = std::make_shared<ModelData>();
	newData->m_name.assign(name.begin(), name.end());
	newData->m_RootNode = std::make_shared<Node>();
	newData->RS = m_ResourceManager.lock()->Get<RenderState>(L"Solid");

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		ProcessMesh(newData, (scene->mMeshes[i]), i, filter);
	}

	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		ProcessMaterials(newData, scene->mMaterials[i]);
	}

	if (Filter::SKINNING == filter)
	{
		ProcessNode(nullptr, newData->m_RootNode, scene->mRootNode, newData->m_Meshes,newData->m_Nodes);
		ProcessBoneNodeMapping(newData);
	}

	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		ProcessAnimation(newData, scene->mAnimations[i]);
	}

	std::wstring key;
	key.assign(name.begin()/*+ 12*/, name.end());
	newData->UID = UID;
	m_ResourceManager.lock()->Add<ModelData>(key, newData);
	//SaveBoneDataTexture(newData);

}

void ModelLoader::SaveBoneDataTexture(std::shared_ptr<ModelData> newData)
{
	//model bonedata를 texture에 저장하자
	//이미 계산된 행렬을 보간하는 작업을 gpu에 넘겨버리면
	//instancing을 할때 buffer에 담을 데이터 수가 적어진다

	//여기서 각 애니메이션에 대한 정보값을 가지고 있어야한다

	//각각의 애니메이션 순회
	for (auto& Animation : newData->m_Animations)
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = 16; // 4 rows for each bone matrix
		texDesc.Height = Animation->m_Channels.size() * 2/*totals.size*/; // 각 인스턴스마다 본 데이터를 포함
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		for (auto& channel : Animation->m_Channels)
		{
			//해당 애니메이션의 각 본의 프레임당 계산된 위치

			//save totals (animation value)
			std::vector<DirectX::XMFLOAT4> textureData(Animation->m_Channels.size());
			for (int i = 0; i < channel->totals.size(); i++)
			{
				DirectX::SimpleMath::Matrix mat = channel->totals[i].second;  // 실제 애니메이션 본 행렬로 대체

				textureData[i * 4 + 0] = DirectX::XMFLOAT4(mat.m[0]);
				textureData[i * 4 + 1] = DirectX::XMFLOAT4(mat.m[1]);
				textureData[i * 4 + 2] = DirectX::XMFLOAT4(mat.m[2]);
				textureData[i * 4 + 3] = DirectX::XMFLOAT4(mat.m[3]);
			}
		}


		D3D11_SUBRESOURCE_DATA initData = {};
		//initData.pSysMem = textureData.data();
		initData.SysMemPitch = texDesc.Width * sizeof(DirectX::XMFLOAT4);

		//create texture
		ID3D11Texture2D* boneTexture;
		HRESULT hr = m_Device.lock()->Get()->CreateTexture2D(&texDesc, &initData, &boneTexture);


		//save texture to .dds
		DirectX::ScratchImage image;
		hr = DirectX::CaptureTexture(m_Device.lock()->Get(), m_Device.lock()->Context(), boneTexture, image);
		if (SUCCEEDED(hr))
		{
			std::wstring path = L"..//Data//Texture//";

			std::wstring finalpath = path + newData->m_name + L".dds";	//어떤 애니메이션인지 구분하기 위한 index 필요

			hr = DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, finalpath.c_str());
			if (FAILED(hr)) {
				// 파일 저장 실패 처리
			}
		}

	}
}

//메쉬 저장
void ModelLoader::ProcessMesh(std::shared_ptr<ModelData> Model, aiMesh* mesh, unsigned int index, Filter filter)
{
	std::wstring str_index = std::to_wstring(index);

	aiMesh* curMesh = mesh;

	std::shared_ptr<Mesh> newMesh;
	switch (filter)
	{
		case Filter::STATIC:
			newMesh = std::make_shared<StaticMesh>();
			break;
		case Filter::SKINNING:
			newMesh = std::make_shared<SkinnedMesh>();
			break;
	}

	newMesh->MinBounding = { mesh->mAABB.mMin.x, mesh->mAABB.mMin.y,mesh->mAABB.mMin.z };
	newMesh->MaxBounding = { mesh->mAABB.mMax.x, mesh->mAABB.mMax.y,mesh->mAABB.mMax.z };

	newMesh->m_primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	newMesh->m_material = mesh->mMaterialIndex;

	///메쉬에따라 읽는방식과 세부 설정을 따로 둬야할듯
	std::vector<BaseVertex> TextureVertices;
	std::vector<SkinningVertex> SkinningVertices;

	D3D11_BUFFER_DESC desc = Vertex::Default;
	D3D11_SUBRESOURCE_DATA data;

	std::vector<UINT> indexbuffer;
	for (unsigned int i = 0; i < curMesh->mNumFaces; i++)
	{
		ProcessIndexBuffer(indexbuffer, curMesh->mFaces + i);
	}

	D3D11_BUFFER_DESC idesc = Index::Default;
	idesc.ByteWidth = sizeof(UINT) * curMesh->mNumFaces * 3;

	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &(indexbuffer[0]);

	//face 당 3개의 인덱스 사용 - 삼각형으로 면을 구성하기때문
	newMesh->m_IB = m_ResourceManager.lock()->Create<IndexBuffer>(Model->m_name + L"_" + str_index + L"_IB", idesc, idata, curMesh->mNumFaces * 3);

	switch (filter)
	{
		case Filter::STATIC:
		{


			//유니티랑 기준을 맞추려고 회전했으므로 바운딩볼륨도 회전시켜놔야지
			DirectX::SimpleMath::Matrix test;
			DirectX::SimpleMath::Vector3 v;

			test._11 = 1; test._12 = 0; test._13 = 0;
			test._21 = 0; test._22 = 0; test._23 = 1;
			test._31 = 0; test._32 = -1; test._33 = 0;

			v = newMesh->MaxBounding;
			v = XMVector3Transform(v, test);
			newMesh->MaxBounding = { v.x,v.y,v.z };

			float maxL = v.Length();

			v = newMesh->MinBounding;
			v = XMVector3Transform(v, test);

			float minL = v.Length();
			if (minL < maxL)
			{
				newMesh->MinBounding = { v.x,v.y,v.z };
			}
			else
			{
				newMesh->MinBounding = newMesh->MaxBounding;
				newMesh->MaxBounding = { v.x,v.y,v.z };
			}

			//각 메쉬에대한 pivot
			newMesh->Pivot = DirectX::SimpleMath::Vector3(newMesh->MaxBounding.x + newMesh->MinBounding.x, newMesh->MaxBounding.y + newMesh->MinBounding.y, newMesh->MaxBounding.z + newMesh->MinBounding.z) / 2;
			//특정 메쉬를 기준으로 각각의 위치를 맞춰야 메쉬를 통으로 그렸을때 맞음
			if (index < 1)
			{
				Model->Pivot = newMesh->Pivot;
			}

			for (unsigned int i = 0; i < curMesh->mNumVertices; i++)
			{
				ProcessVertexBuffer(TextureVertices, curMesh, i);

				DirectX::SimpleMath::Vector3 curPos;
				curPos.x = TextureVertices.back().pos.x;
				curPos.y = TextureVertices.back().pos.y;
				curPos.z = TextureVertices.back().pos.z;

				curPos -= Model->Pivot;

				Model->vertices.push_back(curPos);
			}

			for (auto& ver : TextureVertices)
			{
				DirectX::XMFLOAT4& curPos = ver.pos;
				curPos.x -= Model->Pivot.x;
				curPos.y -= Model->Pivot.y;
				curPos.z -= Model->Pivot.z;
			}
			/*
			*/


			desc.ByteWidth = sizeof(BaseVertex) * curMesh->mNumVertices;
			data.pSysMem = &(TextureVertices[0]);
			newMesh->m_VB = m_ResourceManager.lock()->Create<VertexBuffer>(Model->m_name + L"_" + str_index + L"_VB", desc, data, sizeof(BaseVertex));
		}
		break;

		case Filter::SKINNING:
		{

			for (unsigned int i = 0; i < curMesh->mNumVertices; i++)
			{
				ProcessVertexBuffer(SkinningVertices, curMesh, i);
			}

			std::shared_ptr<SkinnedMesh> newSkin = std::dynamic_pointer_cast<SkinnedMesh>(newMesh);

			//process bone
			for (unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* curAiBone = mesh->mBones[i];

				std::shared_ptr<Bone> curBone = std::make_shared<Bone>();
				std::string name = curAiBone->mName.C_Str();
				curBone->name.assign(name.begin(), name.end());
				curBone->Boneindex = i;

				newSkin->m_BoneData.push_back(curBone);

				DirectX::XMFLOAT4X4 temp;

				for (int j = 0; j < static_cast<int>(curAiBone->mNumWeights); j++)
				{
					curBone->weights.push_back(curAiBone->mWeights[j].mWeight);
					curBone->vertexids.push_back(curAiBone->mWeights[j].mVertexId);
				}

				temp._11 = curAiBone->mOffsetMatrix.a1;
				temp._12 = curAiBone->mOffsetMatrix.a2;
				temp._13 = curAiBone->mOffsetMatrix.a3;
				temp._14 = curAiBone->mOffsetMatrix.a4;

				temp._21 = curAiBone->mOffsetMatrix.b1;
				temp._22 = curAiBone->mOffsetMatrix.b2;
				temp._23 = curAiBone->mOffsetMatrix.b3;
				temp._24 = curAiBone->mOffsetMatrix.b4;

				temp._31 = curAiBone->mOffsetMatrix.c1;
				temp._32 = curAiBone->mOffsetMatrix.c2;
				temp._33 = curAiBone->mOffsetMatrix.c3;
				temp._34 = curAiBone->mOffsetMatrix.c4;

				temp._41 = curAiBone->mOffsetMatrix.d1;
				temp._42 = curAiBone->mOffsetMatrix.d2;
				temp._43 = curAiBone->mOffsetMatrix.d3;
				temp._44 = curAiBone->mOffsetMatrix.d4;

				curBone->offsetMatrix = DirectX::XMLoadFloat4x4(&temp);
				curBone->offsetMatrixInverse = curBone->offsetMatrix.Invert();
			}

			ProcessBoneMapping(SkinningVertices, curMesh, newMesh);
			desc.ByteWidth = sizeof(SkinningVertex) * curMesh->mNumVertices;
			data.pSysMem = &(SkinningVertices[0]);
			newMesh->m_VB = m_ResourceManager.lock()->Create<VertexBuffer>(Model->m_name + L"_" + str_index + L"_VB", desc, data, sizeof(SkinningVertex));
		}
		break;
		case Filter::END:
			break;
		default:
			break;
	}

	Model->m_Meshes.push_back(newMesh);
}
//머테리얼 저장
void ModelLoader::ProcessMaterials(std::shared_ptr<ModelData> Model, aiMaterial* material)
{
	std::shared_ptr<Material> newMaterial = std::make_shared<Material>(m_Device.lock());

	// Diffuse
	const std::wstring basePath = L"..\\Data\\FBX\\";
	std::filesystem::path path;
	std::wstring finalPath;
	std::string name = material->GetName().C_Str();

	std::vector<std::pair<aiTextureType, std::string>> textureProperties;

	int max_size = aiTextureType_TRANSMISSION + 1;
	textureProperties.resize(max_size);

	for (size_t i = 0; i < max_size; i++)
	{
		textureProperties[i].first = (aiTextureType)i;
		aiString texturePath;
		if (AI_SUCCESS == material->GetTexture((aiTextureType)i, 0, &texturePath))
		{
			std::string temp = (texturePath.C_Str());
			bool is_valid = true;
			for (char c : temp)
			{
				if (c < 32 || c > 126) {
					is_valid = false;

					std::wstring wfilename;
					wfilename.assign(Model->m_name.begin(), Model->m_name.end());

					std::wstring texturename;

					switch ((aiTextureType)i)
					{
						case aiTextureType_DIFFUSE:
							texturename = L" Diffuse";
							break;

						case aiTextureType_NORMALS:
							texturename = L" Normal";
							break;

						case aiTextureType_SPECULAR:
							texturename = L" Specular";
							break;

						case aiTextureType_EMISSIVE:
							texturename = L" Emissive";
							break;

						case aiTextureType_METALNESS:
							texturename = L" Metalic";
							break;
						case aiTextureType_SHININESS:
							texturename = L" Roughness";
							break;

						case aiTextureType_AMBIENT_OCCLUSION:
							texturename = L" AO";
							break;


						default:
							break;
					}

					std::string filePathStr = Util::ToMultiByte(wfilename + texturename);

					//wfilename = L"Error loading files : " + wfilename + texturename + L" path is not valid";
					//LPCWSTR name = wfilename.c_str();
					//MessageBox(0, name, 0, 0);
					break;
				}
			}

			if (is_valid)
			{
				path = std::string(texturePath.C_Str());
				textureProperties[i].second = path.filename().string();
			}
		}
	}

	aiColor3D color(1.f, 1.f, 1.f);
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
	{
		//m_Color ={ color.r, color.g, color.b , 1};
	}

	///Albedo
	path = (textureProperties[aiTextureType_DIFFUSE].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		newMaterial->AlbeoPath = finalPath;

		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->m_AlbedoSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->m_Data.useAMRO.x += 1;
	}
	else
	{
		finalPath = basePath + L"base.png";
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		newMaterial->AlbeoPath = finalPath;

		newMaterial->m_AlbedoSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(L"base.png", L"base.png");
	}

	///Normal
	path = (textureProperties[aiTextureType_NORMALS].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		newMaterial->NormalPath = finalPath;
		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->m_NormalSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->m_Data.useNEOL.x += true;

		//m_pNormal = ResourceManager::Instance->CreateTextureResource(finalPath);
		//m_MaterialMapFlags |= MaterialMapFlags::NORMAL;
	}
	else
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		newMaterial->NormalPath = finalPath;
		newMaterial->m_NormalSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(L"base.png", L"base.png");

	}

	path = (textureProperties[aiTextureType_SPECULAR].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		//newMaterial->m_SpecularFilePath = finalPath;
		//newMaterial->m_SpecularSRV->Load(finalPath);
		//m_pSpecular = ResourceManager::Instance->CreateTextureResource(finalPath);
		//m_MaterialMapFlags |= MaterialMapFlags::SPECULAR;
	}

	path = (textureProperties[aiTextureType_EMISSIVE].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";

		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->m_EmissiveSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->EmissivePath = finalPath;
		newMaterial->m_Data.useNEOL.y += true;
	}

	path = (textureProperties[aiTextureType_OPACITY].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->m_OpacitySRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->OpacityPath = finalPath;
		newMaterial->m_Data.useNEOL.z += true;
		//m_pOpacity = ResourceManager::Instance->CreateTextureResource(finalPath);
		//m_MaterialMapFlags |= MaterialMapFlags::OPACITY;
	}

	///Metalic
	path = (textureProperties[aiTextureType_METALNESS].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->MetalicPath = finalPath;
		newMaterial->m_MetalicSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->m_Data.useAMRO.y += 1;
	}

	///Roughness
	path = (textureProperties[aiTextureType_SHININESS].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->RoughnessPath = finalPath;
		newMaterial->m_RoughnessSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->m_Data.useAMRO.z += 1;

	}

	///AO
	path = (textureProperties[aiTextureType_AMBIENT_OCCLUSION].second);
	if (!path.empty())
	{
		finalPath = basePath + path.filename().wstring();
		finalPath = finalPath.substr(0, finalPath.find_last_of(L"."));
		finalPath += L".dds";
		std::wstring newPath = path.filename().wstring();
		newPath = newPath.substr(0, newPath.find_last_of(L"."));
		newPath += L".dds";
		path = newPath;
		newMaterial->AOPath = finalPath;
		newMaterial->m_AOSRV = m_ResourceManager.lock()->Create<ShaderResourceView>(path.filename().wstring(), path);
		newMaterial->m_Data.useAMRO.w += 1;
	}

	Model->m_Materials.push_back(newMaterial);
}
void ModelLoader::ProcessBoneNodeMapping(std::shared_ptr<ModelData> Model)
{
	for (auto& mesh : Model->m_Meshes)
	{
		if (mesh->IsSkinned())
		{
			std::shared_ptr<SkinnedMesh> curMesh = dynamic_pointer_cast<SkinnedMesh>(mesh);
			for (auto& bone : curMesh->m_BoneData)
			{
				std::shared_ptr<Node> temp = FindNode(bone->name, Model->m_RootNode);
				if (temp != nullptr)
				{
					bone->node = temp;
				}
			}
		}
	}
}
void ModelLoader::ProcessAnimation(std::shared_ptr<ModelData> Model, aiAnimation* animation)
{
	std::shared_ptr<Animation> _Animation = std::make_shared<Animation>();
	_Animation->m_Duration = animation->mDuration;
	_Animation->m_TickFrame = animation->mTicksPerSecond;

	for (unsigned int i = 0; i < animation->mNumChannels; i++)
	{
		_Animation->m_Channels.push_back(std::make_shared<Channel>());

		std::shared_ptr<Channel> ob_Channel = _Animation->m_Channels.back();
		aiNodeAnim* curChannel = animation->mChannels[i];

		const char* name = curChannel->mNodeName.C_Str();
		ob_Channel->nodename = std::wstring(name, name + curChannel->mNodeName.length);
		ob_Channel->node = FindNode(ob_Channel->nodename, Model->m_RootNode);

		double prevTime = 0;

		for (unsigned int j = 0; j < curChannel->mNumPositionKeys; j++)
		{
			aiVectorKey curAnimation = curChannel->mPositionKeys[j];
			//if (curAnimation.mTime - prevTime > 0.05)
			{
				ob_Channel->positionkey.push_back(std::make_shared<Key>());

				std::shared_ptr<Key> curKey = ob_Channel->positionkey.back();

				curKey->time = curAnimation.mTime;

				curKey->value.x = curAnimation.mValue.x;
				curKey->value.y = curAnimation.mValue.y;
				curKey->value.z = curAnimation.mValue.z;

			}
			prevTime = curAnimation.mTime;
		}

		prevTime = 0;

		for (unsigned int j = 0; j < curChannel->mNumScalingKeys; j++)
		{
			aiVectorKey curAnimation = curChannel->mScalingKeys[j];
			//if (curAnimation.mTime - prevTime > 0.05)
			{
				ob_Channel->scalingkey.push_back(std::make_shared<Key>());

				std::shared_ptr<Key> curKey = ob_Channel->scalingkey.back();

				curKey->time = curAnimation.mTime;

				curKey->value.x = curAnimation.mValue.x;
				curKey->value.y = curAnimation.mValue.y;
				curKey->value.z = curAnimation.mValue.z;
			}
			prevTime = curAnimation.mTime;

		}

		prevTime = 0;
		for (unsigned int j = 0; j < curChannel->mNumRotationKeys; j++)
		{
			aiQuatKey curAnimation = curChannel->mRotationKeys[j];
			//if (curAnimation.mTime - prevTime > 0.05)
			{
				ob_Channel->rotationkey.push_back(std::make_shared<Key>());

				std::shared_ptr<Key> curKey = ob_Channel->rotationkey.back();

				curKey->time = curAnimation.mTime;

				curKey->rotation.x = curAnimation.mValue.x;
				curKey->rotation.y = curAnimation.mValue.y;
				curKey->rotation.z = curAnimation.mValue.z;
				curKey->rotation.w = curAnimation.mValue.w;
			}
			prevTime = curAnimation.mTime;
		}

		//최종 애니메이션 행렬 연산
		for (unsigned int j = 0; j < curChannel->mNumRotationKeys; j++)
		{
			DirectX::SimpleMath::Matrix rotation{};
			DirectX::SimpleMath::Matrix translate{};
			DirectX::SimpleMath::Matrix scale{};

			translate = DirectX::SimpleMath::Matrix::CreateTranslation(ob_Channel->positionkey[j]->value);

			scale = DirectX::SimpleMath::Matrix::CreateScale(ob_Channel->scalingkey[j]->value);

			rotation = DirectX::SimpleMath::Matrix::CreateFromQuaternion(ob_Channel->rotationkey[j]->rotation);

			DirectX::SimpleMath::Matrix total = scale * rotation * translate;

			ob_Channel->totals.push_back({ curChannel->mRotationKeys[j].mTime, total });
		}

	}

	Model->m_Animations.push_back(_Animation);
}


//버텍스버퍼 저장
void ModelLoader::ProcessVertexBuffer(std::vector<SkinningVertex>& buffer, aiMesh* curMesh, unsigned int index)
{
	SkinningVertex vertex;

	vertex.pos.x = curMesh->mVertices[index].x;
	vertex.pos.y = curMesh->mVertices[index].y;
	vertex.pos.z = curMesh->mVertices[index].z;
	vertex.pos.w = 1;

	vertex.color = { 0.7f,0.7f, 0.7f, 1.0f };

	vertex.normal.x = curMesh->mNormals[index].x;
	vertex.normal.y = curMesh->mNormals[index].y;
	vertex.normal.z = curMesh->mNormals[index].z;

	vertex.tangent.x = curMesh->mTangents[index].x;
	vertex.tangent.y = curMesh->mTangents[index].y;
	vertex.tangent.z = curMesh->mTangents[index].z;

	vertex.bitangent.x = curMesh->mBitangents[index].x;
	vertex.bitangent.y = curMesh->mBitangents[index].y;
	vertex.bitangent.z = curMesh->mBitangents[index].z;

	vertex.TexCord.x = curMesh->mTextureCoords[0][index].x;
	vertex.TexCord.y = curMesh->mTextureCoords[0][index].y;

	//bone data size
	for (int i = 0; i < 8; i++)
	{
		vertex.BoneIndices[i] = 0.f;
		vertex.BoneWeights[i] = 0.f;
	}
	buffer.push_back(vertex);
}
void ModelLoader::ProcessVertexBuffer(std::vector<BaseVertex>& buffer, aiMesh* curMesh, unsigned int index)
{
	BaseVertex vertex;


	vertex.color = { 0.7f,0.7f, 0.7f, 1.0f };

	//유니티랑 x축 90 차이나서 의도적으로 회전시킴
	//왼손좌표계라 기존 회전방향을 반대로 곱해준다
	/*
	*/

	DirectX::SimpleMath::Matrix _RotX90;

	vertex.pos.x = curMesh->mVertices[index].x;
	vertex.pos.y = curMesh->mVertices[index].y;
	vertex.pos.z = curMesh->mVertices[index].z;
	vertex.pos.w = 1;

	_RotX90._11 = 1; _RotX90._12 = 0; _RotX90._13 = 0;
	_RotX90._21 = 0; _RotX90._22 = 0; _RotX90._23 = 1;
	_RotX90._31 = 0; _RotX90._32 = -1; _RotX90._33 = 0;

	DirectX::SimpleMath::Vector3 v = { vertex.pos.x,vertex.pos.y,vertex.pos.z };
	v = XMVector3Transform(v, _RotX90);
	vertex.pos = { v.x,v.y,v.z,1 };

	vertex.normal.x = curMesh->mNormals[index].x;
	vertex.normal.y = curMesh->mNormals[index].y;
	vertex.normal.z = curMesh->mNormals[index].z;

	v = { vertex.normal.x,vertex.normal.y,vertex.normal.z };
	v = XMVector3Transform(v, _RotX90);
	vertex.normal = { v.x,v.y,v.z,0 };

	vertex.tangent.x = curMesh->mTangents[index].x;
	vertex.tangent.y = curMesh->mTangents[index].y;
	vertex.tangent.z = curMesh->mTangents[index].z;

	v = { vertex.tangent.x,vertex.tangent.y,vertex.tangent.z };
	v = XMVector3Transform(v, _RotX90);
	vertex.tangent = { v.x,v.y,v.z,0 };

	vertex.bitangent.x = curMesh->mBitangents[index].x;
	vertex.bitangent.y = curMesh->mBitangents[index].y;
	vertex.bitangent.z = curMesh->mBitangents[index].z;

	v = { vertex.bitangent.x,vertex.bitangent.y,vertex.bitangent.z };
	v = XMVector3Transform(v, _RotX90);
	vertex.bitangent = { v.x,v.y,v.z,0 };

	//texture uv channel
	vertex.TexCord.x = curMesh->mTextureCoords[0][index].x;
	vertex.TexCord.y = curMesh->mTextureCoords[0][index].y;

	//lightmap uv channel
	if (curMesh->mTextureCoords[1] != nullptr)
	{
		vertex.LightMapUV.x = curMesh->mTextureCoords[1][index].x;
		vertex.LightMapUV.y = curMesh->mTextureCoords[1][index].y;
	}
	buffer.push_back(vertex);
}

//인덱스버퍼 저장
void ModelLoader::ProcessIndexBuffer(std::vector<UINT>& buffer, aiFace* curFace)
{
	for (unsigned int i = 0; i < curFace->mNumIndices; i++)
	{
		buffer.push_back(curFace->mIndices[i]);
	}
}

void ModelLoader::ProcessNode(std::shared_ptr<Node> parents, std::shared_ptr<Node> ob_node, aiNode* node, std::vector<std::shared_ptr<Mesh>>& meshes, std::vector<std::shared_ptr<Node>>&nodes ,int index/*=0*/)
{
	std::string Name = node->mName.C_Str();
	ob_node->name.assign(Name.begin(), Name.end());
	nodes.push_back(ob_node);

	//local
	DirectX::XMFLOAT4X4 temp;
	temp._11 = node->mTransformation.a1;
	temp._12 = node->mTransformation.a2;
	temp._13 = node->mTransformation.a3;
	temp._14 = node->mTransformation.a4;

	temp._21 = node->mTransformation.b1;
	temp._22 = node->mTransformation.b2;
	temp._23 = node->mTransformation.b3;
	temp._24 = node->mTransformation.b4;

	temp._31 = node->mTransformation.c1;
	temp._32 = node->mTransformation.c2;
	temp._33 = node->mTransformation.c3;
	temp._34 = node->mTransformation.c4;

	temp._41 = node->mTransformation.d1;
	temp._42 = node->mTransformation.d2;
	temp._43 = node->mTransformation.d3;
	temp._44 = node->mTransformation.d4;

	ob_node->m_Local = DirectX::XMLoadFloat4x4(&temp);
	//assimp는 row major	
	/*
		X1  Y1  Z1  T1
		X2  Y2  Z2  T2
		X3  Y3  Z3  T3
		0   0   0   1

	*/


	ob_node->m_LocalInverse = ob_node->m_Local.Invert();
	ob_node->index = index;

	if (parents != nullptr)
	{
		ob_node->HasParents = true;
		ob_node->m_Parents = parents;
		ob_node->parentsindex = parents->index;

		//이게 local로써 constantbuffer에 들어가야함 이게 계속 바뀌어야함 물론 원본이 아니라 밖에서 이걸 이용해 만든 것들 그러면 이걸 상수 버퍼로 만들어버려
		ob_node->m_World = (nodes[ob_node->parentsindex]->m_World * ob_node->m_Local);
		ob_node->m_WorldInverse = ob_node->m_World.Invert();
	}
	else
	{
		ob_node->m_World = ob_node->m_Local;
		ob_node->m_WorldInverse = ob_node->m_World.Invert();
		ob_node->parentsindex = index;
	}

	if (node->mNumMeshes != 0)
	{
		//ob_node->m_Meshes.resize(node->mNumMeshes);
		int meshindex = *node->mMeshes;

		ob_node->m_Meshes.push_back(meshes[meshindex]);

		for (auto& mesh : ob_node->m_Meshes)
		{
			std::shared_ptr<SkinnedMesh> curMesh = std::dynamic_pointer_cast<SkinnedMesh>(mesh.lock());
			curMesh->m_node = ob_node;
		}
	}
	
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ob_node->m_Childs.push_back(std::make_shared<Node>());
		ProcessNode(ob_node, ob_node->m_Childs.back(), node->mChildren[i], meshes, nodes,++index);
	}
}

void ModelLoader::ProcessBoneMapping(std::vector<SkinningVertex>& buffer, aiMesh* curAiMesh, std::shared_ptr<Mesh> curMesh)
{

	std::shared_ptr<SkinnedMesh> curSkin = std::dynamic_pointer_cast<SkinnedMesh>(curMesh);

	UINT meshBoneCount = curSkin->m_BoneData.size();


	for (UINT i = 0; i < meshBoneCount; ++i)
	{
		std::shared_ptr<Bone> curBone = curSkin->m_BoneData[i];

		//버텍스 버퍼 만들기위해 버텍스 내용채움
		for (int j = 0; j < curBone->vertexids.size(); j++)
		{
			int curVertexId = curBone->vertexids[j];
			float curWeight = curBone->weights[j];

			//i가 현재 본의 인덱스 
			int size = 8;

			if (buffer[curVertexId].BoneIndices[0] == 0.f ||
				buffer[curVertexId].BoneIndices[1] == 0.f ||
				buffer[curVertexId].BoneIndices[2] == 0.f ||
				buffer[curVertexId].BoneIndices[3] == 0.f ||
				buffer[curVertexId].BoneIndices[4] == 0.f ||
				buffer[curVertexId].BoneIndices[5] == 0.f ||
				buffer[curVertexId].BoneIndices[6] == 0.f ||
				buffer[curVertexId].BoneIndices[7] == 0.f
				)
			{

				for (int j = 0; j < size; j++)
				{
					if (buffer[curVertexId].BoneIndices[j] == 0.0f)
					{
						buffer[curVertexId].BoneIndices[j] = static_cast<float>(i);
						break;
					}
				}
			}


			if (buffer[curVertexId].BoneWeights[0] == 0.0f ||
				buffer[curVertexId].BoneWeights[1] == 0.0f ||
				buffer[curVertexId].BoneWeights[2] == 0.0f ||
				buffer[curVertexId].BoneWeights[3] == 0.0f ||
				buffer[curVertexId].BoneWeights[4] == 0.0f ||
				buffer[curVertexId].BoneWeights[5] == 0.0f ||
				buffer[curVertexId].BoneWeights[6] == 0.0f ||
				buffer[curVertexId].BoneWeights[7] == 0.0f
				)
			{
				for (int j = 0; j < size; j++)
				{
					if (buffer[curVertexId].BoneWeights[j] == 0.0f)
					{
						buffer[curVertexId].BoneWeights[j] = curWeight;
						break;
					}
				}
			}
		}

	}
	int a = 3;
}

std::shared_ptr<Node> ModelLoader::FindNode(std::wstring nodename, std::shared_ptr<Node> RootNode)
{
	//탐색방법도 일단 중요할듯

	if (RootNode->name == nodename)
	{
		return RootNode;
	}
	else
	{
		if (!RootNode->m_Childs.empty())
		{

			for (auto& node : RootNode->m_Childs)
			{
				std::shared_ptr<Node> temp = FindNode(nodename, node);

				if (temp != nullptr)
				{
					return temp;
				}
			}

		}

		return nullptr;
	}
}
