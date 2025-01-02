#include "pch.h"
#include "TextObject.h"

TextObject::TextObject(uint32_t entityID, const ui::TextInfo& info)
	: m_ID(entityID), m_Info(info)
{
}
