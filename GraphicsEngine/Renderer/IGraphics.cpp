#pragma once
#include "pch.h"
#include "IGraphics.h"
#include "Graphics.h"

IGraphics* CreateInstance(HWND hWnd)
{
	return new Graphics(hWnd);
}

void DestroyInstance(IGraphics* instance)
{
	delete instance;
}

IGraphics::~IGraphics()
{

}
