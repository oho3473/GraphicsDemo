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
/// 렌더링 파이프라인 단위
///Draw Call하는 하나의 실행 단위
///무언가 그리기위해 바인딩하고 DrawPrimitiveCall하는 일련의 과정
///inputlay + setVS + setPS …… + drawindexed까지의 과정
/// 
/// 기본적인 패스 종류 forward, deferred
/// vs ps에따라 그려지는게 다른데 필요한 cb도 달라짐
/// 메쉬에 따라 texture도 다른거 누군가는 texture가 필요없다
/// 지금 object가 이걸 다 가지고 있단 말이지? 근데 같은 오브젝트에 붙은 mesh여도 표현이 다를거란 말이야? 그러면 메쉬단위로 패스를 타야하는데
/// 서로 다른 패스에 알맞게 들어가야할 cb texture를 어떻게? 맞춰 넣어줄까 메쉬를 넣어줄떄 한쌍이 될 cb texture를 같이 보내줘야지
/// 근데 이게 지금 object가 가지고 있는 곳에서하는거랑 뭐가 다를까? 물론 같은 모델에 texture만 바꿔서 한다고 할때는 따로 세팅하는게 맞는데
/// 뭔줄알고 셋팅해주냐고 -> 각 패스의 고유한 자원들을 어떻게 할거냐 ->일단 각 패스별로 만든다? 패스가 늘어날 수록 클래스가 계속 생기겠지? 패스가 그렇게 많이 늘어나긴 할까?
/// 어떤걸 그리기 위해 어떤 패스에 넣어야할지도 필요
/// 메쉬 단위로 그리게 되면 애니메이션은 어떻게 해야할까?
/// 
/// 2024.05.17
/// 만드려고 생각해보니 static, skinned mesh처리가 각각 다른데 render 로직이 메쉬에 따라 바뀜
/// 함수 두개를 만들어? 아니면 클래스르 두개 만들어? 각 패스마다 처리 방식이 다르겠지?
/// 
/// 한마디로 패스를 만들기
/// 주어진 리소스를 통해 다양한 패스 만들기
/// 
/// 
/// 문제가 뭐냐면 ModelData는 fbx에서 읽어온 데이터들인데 여기에 Mesh가 여러개 들어있어
/// 근데 pass에서는 하나의 mesh  그 메쉬에 사용할 texture + cb 데이터가 필요
/// Modeldata 단위로 넣으면 mesh 단위로 그려내기 쉽지 않음
/// 
/// 패스들을 다 모아다 쭉 render만 한다고 치면?
/// 메쉬마다 각 패스의 인스턴스가 생기는디
/// 그게 아니라 패스에 각각의 메쉬들이 대기타고 있다가 그릴건데
/// 오브젝트가 쓰는 cb가 아닌 것들은 외부에서 넣어줘야한다
/// 2024.06.04
/// 패스마다 skinning static을 나눌 필요가 있나? 패스마다가 아니라 저게 패스 그자체임
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

	// 공유하지 않을 데이터 및 함수 === 삭제
	void BindStatic(std::shared_ptr<RenderData> curModel);
	void BindSkeletal(std::shared_ptr<RenderData> curModel, std::shared_ptr<Mesh> mesh);
	std::weak_ptr<PixelShader> m_DebugPS;
	std::weak_ptr<PixelShader> m_MeshPS;
	std::weak_ptr<VertexShader> m_SkeletalMeshVS;
	std::weak_ptr<VertexShader> m_StaticMeshVS;
	std::weak_ptr<RenderTargetView> m_RTV;
	std::weak_ptr<DepthStencilView> m_DSV;
};
