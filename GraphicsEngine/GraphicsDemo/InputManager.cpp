#include "InputManager.h"

void InputManager::Initialize(HWND hwnd)
{
	for (int i = 0; i < 255; i++)
	{
		m_keyState[i] = KEYSTATE::None;
	}

	//���콺 �ʱ�ȭ
	GetCursorPos(&m_prevMousePos);
	ScreenToClient(hwnd, &m_prevMousePos);


	//m_mouse = new DirectX::Mouse();


	m_hwnd = hwnd;
}

void InputManager::Update()
{
	//Ű�����̽� �迭 ũ��
	//0xFE + 1 = 255

	if (GetFocus())
	{
		for (int i = 0; i < 255; i++)
		{
			SHORT keyState = GetAsyncKeyState(i);

			// ������ ��Ʈ�� 1�̸� Ű�� ���� ���� ������
			bool isKeyDown = keyState & 0x8001;

			//�ȴ��ȴ� -> ���ȴ�
			if ((m_keyState[i] == KEYSTATE::None || m_keyState[i] == KEYSTATE::Up) && isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Down;
			}
			//���ȴ� -> ���ȴ�
			else if ((m_keyState[i] == KEYSTATE::Down) && isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Press;
			}
			//���ȴ� -> �ȴ��ȴ�
			else if ((m_keyState[i] == KEYSTATE::Down || m_keyState[i] == KEYSTATE::Press) && !isKeyDown)
			{
				m_keyState[i] = KEYSTATE::Up;
			}
			//�ȴ��ȴ� -> �ȴ��ȴ�
			else if (m_keyState[i] == KEYSTATE::Up && !isKeyDown)
			{
				m_keyState[i] = KEYSTATE::None;
			}
		}

		//���콺 �̵� ��ȭ�� ���
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
