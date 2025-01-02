#pragma once
#include "vpGraphics.h"

class ResourceManager;
class Device;

class TextObject
{
public:
	TextObject() = default;

	TextObject(uint32_t entityID, const ui::TextInfo& info);

	void SetID(uint32_t entityID) { m_ID = entityID; }
	void SetTextInfo(const ui::TextInfo& info) { m_Info = info; }
	uint32_t GetID() const { return m_ID; }
	ui::TextInfo GetInfo() const { return m_Info; }

private:
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<ResourceManager> m_ResourceManager;

	uint32_t m_ID = 0;
	ui::TextInfo m_Info;

};
