// GraphicsDemo.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GraphicsDemo.h"
#include "IGraphics.h"
#include "CameraManager.h"
#include "TimeManager.h"
#include <iostream>



#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GRAPHICSDEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	RECT tempsize{};
	GetClientRect(hWnd, &tempsize);
	float Width = static_cast<float>(tempsize.right - tempsize.left);
	float Height = static_cast<float>(tempsize.bottom - tempsize.top);
	float ratio = Width / Height;

	CameraManager* cameraManager = new CameraManager(ratio);
	cameraManager->Initialize();

	TimeManager* timeManager = new TimeManager();
	timeManager->Initialize();


	IGraphics* graphicsEngine = CreateGraphics(hWnd);
	graphicsEngine->Initialize();
	graphicsEngine->DebugRenderONOFF(true);


	///모델
	std::shared_ptr<RenderData> test = std::make_shared<RenderData>();
	test->EntityID = 1;
	test->FBX = L"pbrtest.fbx"; //이름으로 어떤 모델을 불러올지 지정
	test->world = DirectX::SimpleMath::Matrix::Identity;
	test->offset = { 0,0 };
	test->lightmapindex = 0;
	test->scale = 1;
	test->tiling = { 0,0 };
	test->punchEffect = false;
	test->isSkinned = false;	//모델이 애니메이션을 가지고 있는가?
	test->isPlay = false;		//모델이 애니메이션을 실행하는가?
	test->color = DirectX::XMFLOAT4{ 0,0,0,0 };
	test->preAni = 0;
	test->curAni = 0;
	test->duration = 0;

	double animationtime = 0;
	if (test->isSkinned && test->isPlay)
	{
		animationtime = graphicsEngine->GetDuration(test->FBX, 0);

	}

	//모델 회전을 위한 변수
	double rotation = 0.0f;

	LightData dir;
	dir.direction = DirectX::XMFLOAT3(0, 0, 1);
	dir.type = static_cast<float>(LightType::Direction);

	graphicsEngine->AddRenderModel(test);
	graphicsEngine->AddLight(1001, LightType::Direction, dir);

	InputManager::GetInstance()->Initialize(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHICSDEMO));


	MSG msg;

	

	///디버그 드로우
	debug::GridInfo grid;
	grid.Color = { 1,1,1,1 };
	grid.GridSize = 10;
	grid.XAxis = { 1,0,0 };
	grid.YAxis = { 0,0,1 };
	grid.Origin = { 0,0,0 };
	grid.XDivs = 100;
	grid.YDivs = 100;

	debug::RayInfo XAxis;
	XAxis.Color = { 1,0,0,1 };
	XAxis.Direction= { 10,0,0 };
	XAxis.Normalize = false;

	debug::RayInfo YAxis;
	YAxis.Color = { 0,1,0,1 };
	YAxis.Direction = { 0,10,0 };
	YAxis.Normalize = false;

	debug::RayInfo ZAxis;
	ZAxis.Color = { 0,0,1,1 };
	ZAxis.Direction = { 0,0,10 };
	ZAxis.Normalize = false;


	// 기본 메시지 루프입니다:
	while (true)
	{

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		timeManager->Update(1);

		//물체 y축 회전
		test->world._11 = cos(rotation);		test->world._13 = sin(rotation);
		test->world._31 = -sin(rotation);		test->world._33 = cos(rotation);

		rotation += 0.5f * timeManager->DeltaTime();

		if (test->isSkinned && test->isPlay)
		{
			test->duration += 0.001f;
			if (animationtime <= test->duration)
			{
				test->duration -= static_cast<float>(animationtime);
			}
		}


		InputManager::GetInstance()->Update();
		cameraManager->Update(timeManager->DeltaTime());
		DirectX::SimpleMath::Matrix view = cameraManager->View();
		//view._41 = -10;
		DirectX::SimpleMath::Matrix proj = cameraManager->Proj();
		DirectX::SimpleMath::Matrix ortho = DirectX::SimpleMath::Matrix::Identity;
		graphicsEngine->SetCamera(view, proj, ortho);

		graphicsEngine->CulingUpdate();
		graphicsEngine->AnimationUpdate(timeManager->DeltaTime());
		graphicsEngine->Update(timeManager->DeltaTime());
		graphicsEngine->EndUpdate(timeManager->DeltaTime());

		
		
		graphicsEngine->DrawGrid(grid);
		graphicsEngine->DrawRay(XAxis);
		graphicsEngine->DrawRay(YAxis);
		graphicsEngine->DrawRay(ZAxis);

		graphicsEngine->BeginRender();
		graphicsEngine->Render(timeManager->DeltaTime());
		graphicsEngine->EndRender();
	}

	graphicsEngine->Finalize();
	DestroyGraphics(graphicsEngine);

	return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHICSDEMO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GRAPHICSDEMO);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 메뉴 선택을 구문 분석합니다:
			switch (wmId)
			{
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
