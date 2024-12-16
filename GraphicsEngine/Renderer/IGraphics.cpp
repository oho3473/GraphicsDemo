#pragma once
#include "Graphics.h"

IGraphics* CreateGraphics(HWND hWnd)
{
	return new Graphics(hWnd);
}

void DestroyGraphics(IGraphics* instance)
{
	//delete instance;
}

IGraphics::~IGraphics()
{

}
