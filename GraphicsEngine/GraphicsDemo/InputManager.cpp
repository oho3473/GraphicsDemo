#include "InputManager.h"

void InputManager::Initialize(HWND hwnd)
{
	for (int i = 0; i < 255; i++)
	{
		m_keyState[i] = KEYSTATE::None;
	}

	//마우스 초기화
	GetCursorPos(&m_prevMousePos);
	ScreenToClient(hwnd, &m_prevMousePos);


	//m_mouse = new DirectX::Mouse();


	m_hwnd = hwnd;
}

void InputManager::Update()
{
	//키스테이스 배열 크기
	//0xFE + 1 = 255

	if (GetFocus())
	{
		for (int i = 0; i < 255; i++)
		{
			SHORT keyState = GetAsyncKeyState(i);

			// 마지막 비트가 1이면 키가 현재 눌린 상태임
			bool isKeyDown = keyState & 0x8001;

			//안눌렸다 -> 눌렸다
			if ((m_keyState[i] == KEYSTATE::None || m_keyState[i] == KEYSTATE::Up) && isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Down;
			}
			//눌렸다 -> 눌렸다
			else if ((m_keyState[i] == KEYSTATE::Down) && isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Press;
			}
			//눌렸다 -> 안눌렸다
			else if ((m_keyState[i] == KEYSTATE::Down || m_keyState[i] == KEYSTATE::Press) && !isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Up;
			}
			//안눌렸다 -> 안눌렸다
			else if (m_keyState[i] == KEYSTATE::Up && !isKeyDown)
			{
				m_keyState[i] = KEYSTATE::None;
			}
		}

		//마우스 이동 변화량 계산
		GetCursorPos(&m_curMousePos);
		ScreenToClient(m_hwnd, &m_curMousePos);

		dxdy.x = (m_curMousePos.x - m_prevMousePos.x);
		dxdy.y = (m_curMousePos.y - m_prevMousePos.y);

		m_prevMousePos = m_curMousePos;
	}

	if (m_wheelState != WheelState::None)
	{
		m_wheelState = WheelState::None;
	}
}

void InputManager::ProcessMouseWheel(int value)
{
	if (value > 0)
	{
		m_wheelState = WheelState::Up;
	}
	else if (value < 0)
	{
		m_wheelState = WheelState::Down;
	}
}

void InputManager::SetMousePos(POINT pos)
{
	m_prevMousePos = m_curMousePos;
	m_curMousePos = pos;
}

LONG InputManager::GetDeltMouseX()
{
	return dxdy.x;
}

LONG InputManager::GetDeltMouseY()
{
	return dxdy.y;
}

InputManager::InputManager() : dxdy(),m_curMousePos(),m_prevMousePos(),m_wheelState(WheelState::None),m_hwnd()
{
}
