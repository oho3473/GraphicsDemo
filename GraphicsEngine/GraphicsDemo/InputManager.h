#pragma once
#include "ISingleton.h"
#include <windows.h>


enum class KEYSTATE
{
	None,
	Down,
	Up,
	Press
};

enum class WheelState
{
	None,
	Down,
	Up
};

class InputManager : public ISingleton<InputManager>
{
	friend class ISingleton<InputManager>;
public:

	void Initialize(HWND hwnd);
	void Update();

	KEYSTATE GetKeyState(int key) { return m_keyState[key]; }
	bool IsKeyDown(int key) { return m_keyState[key] == KEYSTATE::Down; }
	bool IsKeyNone(int key) { return m_keyState[key] == KEYSTATE::None; }
	bool IsKeyUp(int key) { return m_keyState[key] == KEYSTATE::Up; }
	bool IsKeyPress(int key) { return m_keyState[key] == KEYSTATE::Press; }

	/*Vector2 GetMousePosition() { return Vector2((float)m_mousePos.x, (float)m_mousePos.y); }
	Vector2 GetMouseDxdy() { return m_mousedxdy; }*/

	WheelState GetWheelState() { return m_wheelState; }
	void ProcessMouseWheel(int value);


	void SetMousePos(POINT pos);
	LONG GetDeltMouseX();
	LONG GetDeltMouseY();

private:
	InputManager();

	KEYSTATE m_keyState[0xFE/*VK_OEM_CLEAR*/ + 1]; //https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes 각각의 모든 키에 대한 상태 배열

	POINT m_curMousePos;
	POINT m_prevMousePos;
	POINT dxdy;

	//Vector2 m_mousedxdy;

	WheelState m_wheelState = WheelState::None;

	HWND m_hwnd;


};

