#pragma once
#include <iostream>
#include <format>

#include "CameraManager.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "..\Renderer\IGraphics.h"

class Process
{
public:
	Process(HWND hWnd);
	~Process();

	void Initialize();
	void Update();
	void Render();
	void Finalize();
	void OnResize(HWND hWnd,bool isFullScreen);
private:
	void SetScene();

private:
	HWND m_hWnd;

private:
	IGraphics* m_graphicsEngine;
	CameraManager* m_cameraManager;
	TimeManager* m_timeManager;
	InputManager* m_inputManager;

	std::vector<std::shared_ptr<RenderData>> m_models;
	std::vector<ui::TextInfo> m_UIs;
	bool DebugOnOff= true;
	bool IBLOnOff= false;
};

