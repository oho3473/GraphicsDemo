#include "pch.h"
#include "UIManager.h"

#include <memory>
#include <memory>
#include <memory>

#include "Device.h"
#include "ResourceManager.h"

#include "TextObject.h"

#include "Util.h"

void UIManager::Initialize(const std::shared_ptr<Device>& device,
	const std::shared_ptr<ResourceManager>& resourceManager)
{
	m_Device = device;
	m_ResourceManager = resourceManager;

	m_SpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_Device->Context());

	m_DefaultFont = std::make_shared<DirectX::SpriteFont>(m_Device->Get(), L"..\\Data\\Font\\KIMM_B48_HY.spritefont");
	m_KIMM32 = std::make_shared<DirectX::SpriteFont>(m_Device->Get(), L"..\\Data\\Font\\KIMM_B32_HY.spritefont");
	m_KIMM48 = std::make_shared<DirectX::SpriteFont>(m_Device->Get(), L"..\\Data\\Font\\KIMM_B48_HY.spritefont");
	m_SpaceShards48 = std::make_shared<DirectX::SpriteFont>(m_Device->Get(), L"..\\Data\\Font\\SpaceShards_48.spritefont");
	m_SpaceShardsItalic48 = std::make_shared<DirectX::SpriteFont>(m_Device->Get(), L"..\\Data\\Font\\SpaceShardsItalic_48.spritefont");
}

void UIManager::Render()
{
	DrawAllTexts();
}

void UIManager::CreateTextObject(uint32_t entityID, const ui::TextInfo& info)
{
	m_Texts.push_back(std::make_shared<TextObject>(entityID, info));
}

void UIManager::UpdateTextObject(uint32_t entityID, const ui::TextInfo& info)
{
	for (const auto& text : m_Texts)
	{
		if (text->GetID() == entityID)
		{
			text->SetTextInfo(info);
		}
	}
}

void UIManager::DeleteTextObject(uint32_t entityId)
{
	auto it = std::remove_if(m_Texts.begin(), m_Texts.end(),
		[entityId](const std::shared_ptr<TextObject>& obj)
		{
			return obj->GetID() == entityId;
		});

	if (it != m_Texts.end())
	{
		m_Texts.erase(it, m_Texts.end());
	}
}


void UIManager::DrawAllTexts()
{
	m_SpriteBatch->Begin();

	uint32_t screenWidth = m_Device->GetWndWidth();
	uint32_t screenHeight = m_Device->GetWndHeight();

	for (const auto& text : m_Texts)
	{
		ui::TextInfo info = text->GetInfo();

		if (info.Color.A() == 0)
			continue;

		const float relPosX = info.PosXPercent / 100.f;
		const float relPosY = info.PosYPercent / 100.f;
		float posX = relPosX * static_cast<float>(screenWidth);
		float posY = relPosY * static_cast<float>(screenHeight);

		auto& font = m_DefaultFont;
		if (info.FontPath == L"KIMM_B48_HY.spritefont")
			font = m_KIMM48;
		else if (info.FontPath == L"KIMM_B32_HY.spritefont")
			font = m_KIMM32;
		else if (info.FontPath == L"SpaceShards_48.spritefont")
			font = m_SpaceShards48;
		else if (info.FontPath == L"SpaceShardsItalic_48.spritefont")
			font = m_SpaceShardsItalic48;
		else
			font = m_DefaultFont;

		auto rotation = info.Angle * (DirectX::XM_PI / 180.f);

		DirectX::SimpleMath::Vector2 origin = font->MeasureString(info.Text.c_str());
		origin.x /= 2.f;
		origin.y /= 2.f;
		font->DrawString(
			m_SpriteBatch.get(),
			info.Text.c_str(),
			DirectX::SimpleMath::Vector2(posX, posY),
			info.Color,
			rotation,
			origin,
			info.Scale,
			DirectX::SpriteEffects_None,
			info.Layer);
	}

	m_SpriteBatch->End();
}
