#pragma once
#include "vpGraphics.h"

#include <SpriteBatch.h>
#include <SpriteFont.h>

/*
 * 게임 UI에 쓰일
 * Image 와 Font를 관리하는 매니저
 */

class TextObject;
class ImageObject;

class UIManager
{
public:
	UIManager() = default;

	void Initialize(const std::shared_ptr<class Device>& device
		, const std::shared_ptr<class ResourceManager>& resourceManager);
	void Render();

	void CreateImageObject(uint32_t entityID, const ui::ImageInfo& info);
	void UpdateImageObject(uint32_t entityID, const ui::ImageInfo& info);
	void DeleteImageObject(uint32_t entityId);

	void CreateTextObject(uint32_t entityID, const ui::TextInfo& info);
	void UpdateTextObject(uint32_t entityID, const ui::TextInfo& info);
	void DeleteTextObject(uint32_t entityId);

	RECT GetImageRect(uint32_t entityID) const;

private: 
	void DrawAllImages();
	void DrawAllTexts();

private:
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<ResourceManager> m_ResourceManager;

	std::vector<std::shared_ptr<ImageObject>> m_Images;

	// TODO: Font
	std::vector<std::shared_ptr<TextObject>> m_Texts;

	// Sprite Font
	std::unique_ptr<DirectX::SpriteBatch> m_SpriteBatch;
	std::shared_ptr<DirectX::SpriteFont> m_SpriteFont;

	std::shared_ptr<DirectX::SpriteFont> m_DefaultFont;
	std::shared_ptr<DirectX::SpriteFont> m_SpaceShards48;
	std::shared_ptr<DirectX::SpriteFont> m_KIMM48;
	std::shared_ptr<DirectX::SpriteFont> m_KIMM32;
	std::shared_ptr<DirectX::SpriteFont> m_SpaceShardsItalic48;
};
 