#pragma once
//#include <directxtk/CommonStates.h>

#include <queue>
#include <vector>
#include <memory>

#include "RenderData.h"
class Device;
class DebugDrawManager;

class ModelData;
struct RenderData;
class Mesh;

class VertexShader;
class PixelShader;
class RenderState;

class LightManager;
class ResourceManager;
class RenderTargetView;
class DepthStencilView;
class RenderTargetView;
class ShaderResourceView;
class IndexBuffer;
class VertexBuffer;

struct D3D11_VIEWPORT;

/// <summary>
/// 2024.05.16
/// ������ ���������� ����
///Draw Call�ϴ� �ϳ��� ���� ����
///���� �׸������� ���ε��ϰ� DrawPrimitiveCall�ϴ� �Ϸ��� ����
///inputlay + setVS + setPS ���� + drawindexed������ ����
/// 
/// �⺻���� �н� ���� forward, deferred
/// vs ps������ �׷����°� �ٸ��� �ʿ��� cb�� �޶���
/// �޽��� ���� texture�� �ٸ��� �������� texture�� �ʿ����
/// ���� object�� �̰� �� ������ �ִ� ������? �ٵ� ���� ������Ʈ�� ���� mesh���� ǥ���� �ٸ��Ŷ� ���̾�? �׷��� �޽������� �н��� Ÿ���ϴµ�
/// ���� �ٸ� �н��� �˸°� ������ cb texture�� ���? ���� �־��ٱ� �޽��� �־��ً� �ѽ��� �� cb texture�� ���� ���������
/// �ٵ� �̰� ���� object�� ������ �ִ� �������ϴ°Ŷ� ���� �ٸ���? ���� ���� �𵨿� texture�� �ٲ㼭 �Ѵٰ� �Ҷ��� ���� �����ϴ°� �´µ�
/// ���پ˰� �������ֳİ� -> �� �н��� ������ �ڿ����� ��� �Ұų� ->�ϴ� �� �н����� �����? �н��� �þ ���� Ŭ������ ��� �������? �н��� �׷��� ���� �þ�� �ұ�?
/// ��� �׸��� ���� � �н��� �־�������� �ʿ�
/// �޽� ������ �׸��� �Ǹ� �ִϸ��̼��� ��� �ؾ��ұ�?
/// 
/// 2024.05.17
/// ������� �����غ��� static, skinned meshó���� ���� �ٸ��� render ������ �޽��� ���� �ٲ�
/// �Լ� �ΰ��� �����? �ƴϸ� Ŭ������ �ΰ� �����? �� �н����� ó�� ����� �ٸ�����?
/// 
/// �Ѹ���� �н��� �����
/// �־��� ���ҽ��� ���� �پ��� �н� �����
/// 
/// 
/// ������ ���ĸ� ModelData�� fbx���� �о�� �����͵��ε� ���⿡ Mesh�� ������ ����־�
/// �ٵ� pass������ �ϳ��� mesh  �� �޽��� ����� texture + cb �����Ͱ� �ʿ�
/// Modeldata ������ ������ mesh ������ �׷����� ���� ����
/// 
/// �н����� �� ��ƴ� �� render�� �Ѵٰ� ġ��?
/// �޽����� �� �н��� �ν��Ͻ��� ����µ�
/// �װ� �ƴ϶� �н��� ������ �޽����� ���Ÿ�� �ִٰ� �׸��ǵ�
/// ������Ʈ�� ���� cb�� �ƴ� �͵��� �ܺο��� �־�����Ѵ�
/// 2024.06.04
/// �н����� skinning static�� ���� �ʿ䰡 �ֳ�? �н����ٰ� �ƴ϶� ���� �н� ����ü��
/// </summary>


class RenderPass
{
public:
	RenderPass() = default;
	virtual ~RenderPass();

	RenderPass(const std::shared_ptr<Device> device, const std::shared_ptr<ResourceManager> resourcemanager);

	virtual void Render() abstract;
	virtual void OnResize() abstract;

	virtual void SetRenderQueue(const std::vector<std::shared_ptr<RenderData>>& renderQueue);

protected:
	void UnBindResource();

	std::weak_ptr<Device> m_Device;
	std::weak_ptr<ResourceManager> m_ResourceManager;

	std::vector<std::shared_ptr<RenderData>> m_RenderList;

	// �������� ���� ������ �� �Լ� === ����
	void BindStatic(std::shared_ptr<RenderData> curModel);
	void BindSkeletal(std::shared_ptr<RenderData> curModel, std::shared_ptr<Mesh> mesh);
	std::weak_ptr<PixelShader> m_DebugPS;
	std::weak_ptr<PixelShader> m_MeshPS;
	std::weak_ptr<VertexShader> m_SkeletalMeshVS;
	std::weak_ptr<VertexShader> m_StaticMeshVS;
	std::weak_ptr<RenderTargetView> m_RTV;
	std::weak_ptr<DepthStencilView> m_DSV;
};
