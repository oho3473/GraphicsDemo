#include "Graphics.h"
#include "Device.h"
#include "ResourceManager.h"
#include "ModelLoader.h"
#include "LightManager.h"
#include "Animator.h"
#include "DecalManager.h"
#include "PassManager.h"




Graphics::Graphics(HWND hWnd) : m_hWnd(hWnd)
, m_wndSize()
, m_Device(std::make_shared<Device>())
, m_ResourceManager(std::make_shared<ResourceManager>())
, m_Loader(std::make_shared <ModelLoader>())
, m_LightManager(std::make_shared<LightManager>())
, m_Animator(std::make_shared <Animator>())
, m_DecalManager(std::make_shared <DecalManager>())
, m_PassManager(std::make_shared <PassManager>()), IGraphics()
{

}

Graphics::~Graphics()
{

}

bool Graphics::Initialize()
{
	return false;
}

void Graphics::CulingUpdate()
{

}

void Graphics::AnimationUpdate(double dt)
{

}

void Graphics::Update(double dt)
{

}

void Graphics::EndUpdate(double dt)
{

}

bool Graphics::Finalize()
{
	return true;
}

void Graphics::BeginRender()
{

}

void Graphics::Render(float deltaTime)
{

}

void Graphics::EndRender()
{

}

void Graphics::OnResize(HWND hwnd, bool isFullScreen)
{

}

void Graphics::DebugRenderONOFF(bool isRender)
{

}

void Graphics::EraseObject(uint32_t EntityID)
{

}

void Graphics::UpdateModel(uint32_t EntityID)
{

}

bool Graphics::AddRenderModel(std::shared_ptr<RenderData> data)
{
	return true;
}

void Graphics::Culling()
{

}

//std::vector<std::shared_ptr<RenderData>>::iterator Graphics::FindEntity(uint32_t id)
//{
//	return ;
//}
