#include "Process.h"

Process::Process(HWND hWnd) : m_hWnd(hWnd), m_inputManager(nullptr), m_timeManager(nullptr), m_cameraManager(nullptr), m_graphicsEngine(nullptr)
{

}

Process::~Process()
{

}

void Process::Initialize()
{
	RECT tempsize{};
	GetClientRect(m_hWnd, &tempsize);
	float Width = static_cast<float>(tempsize.right - tempsize.left);
	float Height = static_cast<float>(tempsize.bottom - tempsize.top);
	float ratio = Width / Height;


	m_cameraManager = new CameraManager(ratio);
	m_cameraManager->Initialize();
	m_cameraManager->SetCurCameraPos({ 0, 1, -3 });

	m_timeManager = new TimeManager();
	m_timeManager->Initialize();

	m_graphicsEngine = CreateGraphics(m_hWnd);
	m_graphicsEngine->Initialize();
	m_graphicsEngine->DebugRenderONOFF(true);

	m_inputManager = InputManager::GetInstance();
	m_inputManager->Initialize(m_hWnd);


	SetScene();
}

void Process::Update()
{

	m_timeManager->Update(m_timeManager->DeltaTime());
	double FPS = m_timeManager->FPS();
	ui::TextInfo fps = m_UIs[1];
	fps.Text = std::format(L"FPS : {:.1f}", FPS);
	m_graphicsEngine->UpdateTextObject(fps.uid, fps);

#pragma region INPUT
	if (InputManager::GetInstance()->IsKeyDown(VK_F3))
	{
		m_graphicsEngine->ChangeDebugQuad(debug::quadstate::PBR);
	}

	if (InputManager::GetInstance()->IsKeyDown(VK_F2))
	{
		m_graphicsEngine->ChangeDebugQuad(debug::quadstate::GEOMETRY);
	}

	if (InputManager::GetInstance()->IsKeyDown(VK_F1))
	{
		DebugOnOff = !DebugOnOff;
		m_graphicsEngine->DebugRenderONOFF(DebugOnOff);
	}

	if (InputManager::GetInstance()->IsKeyDown('1'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->FBX = L"pbrtest.fbx";
	}

	if (InputManager::GetInstance()->IsKeyDown('2'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->FBX = L"monkey.fbx";
	}

	if (InputManager::GetInstance()->IsKeyDown('3'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->FBX = L"vidiger.fbx";
	}

	if (InputManager::GetInstance()->IsKeyDown('4'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->FBX = L"pureMetal.fbx";
	}

	if (InputManager::GetInstance()->IsKeyDown('I'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->useIBL = !testmodel->useIBL;
		IBLOnOff = !IBLOnOff;
		m_graphicsEngine->IBLONOFF(IBLOnOff);
	}

	if (InputManager::GetInstance()->IsKeyDown('O'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->useEditMetalic = !testmodel->useEditMetalic;
		testmodel->metalicRoughness.x = static_cast<INT>(testmodel->useEditMetalic);

	}

	if (InputManager::GetInstance()->IsKeyDown('P'))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->useEditRoughness = !testmodel->useEditRoughness;
		testmodel->metalicRoughness.y = static_cast<INT>(testmodel->useEditRoughness);

	}

	// [{
	if (InputManager::GetInstance()->IsKeyDown(VK_OEM_4))
	{
		std::shared_ptr<RenderData> testmodel = m_models[0];
		testmodel->useEditAlbedo = !testmodel->useEditAlbedo;
		testmodel->albedo.x = static_cast<INT>(testmodel->useEditAlbedo);
	}

	if (InputManager::GetInstance()->IsKeyPress(VK_INSERT))
	{
		if (m_models[0]->useEditMetalic)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->metalicRoughness.z += m_timeManager->DeltaTime();
			if (testmodel->metalicRoughness.z > 1)
			{
				testmodel->metalicRoughness.z = 1;
			}
		}
	}

	if (InputManager::GetInstance()->IsKeyPress(VK_DELETE))
	{
		if (m_models[0]->useEditMetalic)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->metalicRoughness.z -= m_timeManager->DeltaTime();
			if (testmodel->metalicRoughness.z < 0)
			{
				testmodel->metalicRoughness.z = 0;
			}
		}
	}

	if (InputManager::GetInstance()->IsKeyPress(VK_HOME))
	{
		if (m_models[0]->useEditRoughness)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->metalicRoughness.w += m_timeManager->DeltaTime();
			if (testmodel->metalicRoughness.w > 1)
			{
				testmodel->metalicRoughness.w = 1;
			}
		}
	}

	if (InputManager::GetInstance()->IsKeyPress(VK_END))
	{
		if (m_models[0]->useEditRoughness)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->metalicRoughness.w -= m_timeManager->DeltaTime();
			if (testmodel->metalicRoughness.w < 0)
			{
				testmodel->metalicRoughness.w = 0;
			}
		}
	}



	//page up
	if (InputManager::GetInstance()->IsKeyPress(VK_PRIOR))
	{
		if (m_models[0]->useEditAlbedo)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->albedo.y += m_timeManager->DeltaTime();
			testmodel->albedo.z += m_timeManager->DeltaTime();
			testmodel->albedo.w += m_timeManager->DeltaTime();
			if (testmodel->albedo.y > 1)
			{
				testmodel->albedo.y = 1;
				testmodel->albedo.z = 1;
				testmodel->albedo.w = 1;
			}
		}
	}

	//page down
	if (InputManager::GetInstance()->IsKeyPress(VK_NEXT))
	{
		if (m_models[0]->useEditAlbedo)
		{
			std::shared_ptr<RenderData> testmodel = m_models[0];
			testmodel->albedo.y -= m_timeManager->DeltaTime();
			testmodel->albedo.z -= m_timeManager->DeltaTime();
			testmodel->albedo.w -= m_timeManager->DeltaTime();
			if (testmodel->albedo.y < 0)
			{
				testmodel->albedo.y = 0;
				testmodel->albedo.z = 0;
				testmodel->albedo.w = 0;
			}
		}
	}


	if (InputManager::GetInstance()->IsKeyPress(VK_MBUTTON))
	{
		static double rotation = 0.0f;

		std::shared_ptr<RenderData> testmodel = m_models[0];

		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(InputManager::GetInstance()->GetDeltMouseX()));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(InputManager::GetInstance()->GetDeltMouseY()));

		testmodel->world._11 = cosf(rotation);		testmodel->world._13 = sinf(rotation);
		testmodel->world._31 = -sinf(rotation);		testmodel->world._33 = cosf(rotation);
		rotation += 2 * 0.5f * m_timeManager->DeltaTime();
	}
	InputManager::GetInstance()->Update();
#pragma endregion 

	m_cameraManager->Update(m_timeManager->DeltaTime());
	DirectX::SimpleMath::Matrix view = m_cameraManager->View();
	DirectX::SimpleMath::Matrix proj = m_cameraManager->Proj();
	DirectX::SimpleMath::Matrix ortho = DirectX::SimpleMath::Matrix::Identity;

	DirectX::SimpleMath::Vector3 pos = m_cameraManager->GetCamerPos();
	ui::TextInfo camerapos = m_UIs[1];
	camerapos.Text = std::format(L"카메라 위치 : {:.1f}, {:.1f}, {:.1f}", (double)pos.x, (double)pos.y, (double)pos.z);
	m_graphicsEngine->UpdateTextObject(camerapos.uid, camerapos);
	m_graphicsEngine->SetCamera(view, proj, ortho);

	///디버그 드로우
#pragma region GridAxis
	debug::GridInfo grid;
	grid.Color = { 1,1,1,1 };
	grid.GridSize = 100;
	grid.XAxis = { 1,0,0 };
	grid.YAxis = { 0,0,1 };
	grid.Origin = { 0,0,0 };
	grid.XDivs = 100;
	grid.YDivs = 100;

	debug::RayInfo XAxis;
	XAxis.Color = { 1,0,0,1 };
	XAxis.Direction = { 10,0,0 };
	XAxis.Normalize = false;
	XAxis.Origin.y = 0.01;

	debug::RayInfo YAxis;
	YAxis.Color = { 0,1,0,1 };
	YAxis.Direction = { 0,10,0 };
	YAxis.Normalize = false;

	debug::RayInfo ZAxis;
	ZAxis.Color = { 0,0,1,1 };
	ZAxis.Direction = { 0,0,10 };
	ZAxis.Normalize = false;
	ZAxis.Origin.y = 0.01;

	m_graphicsEngine->DrawGrid(grid);
	m_graphicsEngine->DrawRay(XAxis);
	m_graphicsEngine->DrawRay(YAxis);
	m_graphicsEngine->DrawRay(ZAxis);
#pragma endregion

	m_graphicsEngine->Update(m_timeManager->DeltaTime());
}

void Process::Render()
{
	m_graphicsEngine->BeginRender();
	m_graphicsEngine->Render(m_timeManager->DeltaTime());
	m_graphicsEngine->EndRender();
}

void Process::Finalize()
{
	for (auto info : m_UIs)
	{
		m_graphicsEngine->DeleteTextObject(info.uid);
	}

	m_graphicsEngine->Finalize();
	DestroyGraphics(m_graphicsEngine);

}

void Process::OnResize(HWND hWnd, bool isFullScreen)
{
	m_graphicsEngine->OnResize(m_hWnd, isFullScreen);
}

void Process::SetScene()
{

#pragma region Model
	std::shared_ptr<RenderData> testmodel = std::make_shared<RenderData>();
	testmodel->EntityID = 1;
	testmodel->FBX = L"pbrtest.fbx"; //이름으로 어떤 모델을 불러올지 지정
	testmodel->world = DirectX::SimpleMath::Matrix::Identity;
	testmodel->world._42 = 9;
	testmodel->world._43 = -1;
	testmodel->offset = { 0,0 };
	testmodel->lightmapindex = 0;
	testmodel->scale = 1;
	testmodel->tiling = { 0,0 };
	testmodel->isSkinned = false;	//모델이 애니메이션을 가지고 있는가?
	testmodel->isPlay = false;		//모델이 애니메이션을 실행하는가?
	testmodel->albedo = DirectX::XMFLOAT4{ 0,0,0,0 };
	testmodel->preAni = 0;
	testmodel->curAni = 0;
	testmodel->duration = 0;

	double testmodelanimationtime = 0;
	if (testmodel->isSkinned && testmodel->isPlay)
	{
		testmodelanimationtime = m_graphicsEngine->GetDuration(testmodel->FBX, 0);

	}
	m_graphicsEngine->AddRenderModel(testmodel);
	m_models.push_back(testmodel);

	//IBL 비교용 모델
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::shared_ptr<RenderData> testmodel = std::make_shared<RenderData>();
			testmodel->EntityID = (i * 4) + (j+2);
			testmodel->FBX = L"pureMetal.fbx";
			testmodel->world = DirectX::SimpleMath::Matrix::Identity;
			testmodel->world._41 = 2 * (j + 1);
			testmodel->world._42 = 2 * i;
			testmodel->world._43 = 0;
			testmodel->offset = { 0,0 };
			testmodel->lightmapindex = 0;
			testmodel->scale = 1;
			testmodel->tiling = { 0,0 };
			testmodel->isSkinned = false;
			testmodel->isPlay = false;		
			testmodel->albedo = DirectX::XMFLOAT4{ 0,0,0,0 };
			testmodel->preAni = 0;
			testmodel->curAni = 0;
			testmodel->duration = 0;

			testmodel->useIBL = true;
			testmodel->useEditMetalic = true;
			testmodel->useEditRoughness = true;
			testmodel->metalicRoughness = DirectX::XMFLOAT4( 1,1,i * 0.25,j * 0.25 );
			//testmodel->useEditAlbedo = true;
			//testmodel->albedo = DirectX::XMFLOAT4(1, 1, 0,0);
			m_graphicsEngine->AddRenderModel(testmodel);
			m_models.push_back(testmodel);
		}
	}
#pragma endregion 

#pragma region Light
	LightData dir;
	dir.direction = DirectX::XMFLOAT3(0, 0, 1);
	dir.type = static_cast<float>(LightType::Direction);
	m_graphicsEngine->AddLight(1001, LightType::Direction, dir);
#pragma endregion

#pragma region UI
	ui::TextInfo text;
	text.uid = 111;
	text.Color = { 0,1,0,1 };
	text.PosXPercent = 10;
	text.PosYPercent = 3;
	text.Text = L"카메라 이동 : WASD\n카메라 회전 : 마우스 우클릭 이동";
	text.Scale = 0.3f;
	//m_graphicsEngine->CreateTextObject(text.uid, text);
	m_UIs.push_back(text);


	ui::TextInfo camerapos;
	camerapos.uid = 112;
	camerapos.Color = { 1,0,0,1 };
	camerapos.PosXPercent = 9;
	camerapos.PosYPercent = 10;
	camerapos.Text = std::format(L"카메라 위치 : {}, {}, {}", 0, 0, 0);
	camerapos.Scale = 0.3f;
	m_graphicsEngine->CreateTextObject(camerapos.uid, camerapos);
	m_UIs.push_back(camerapos);

	ui::TextInfo fps;
	fps.uid = 113;
	fps.Color = { 1,1,0,1 };
	fps.PosXPercent = 4;
	fps.PosYPercent = 15;
	fps.Text = std::format(L"FPS : {}", 0);
	fps.Scale = 0.3f;
	//m_graphicsEngine->CreateTextObject(fps.uid, fps);
	m_UIs.push_back(fps);
#pragma endregion

}
