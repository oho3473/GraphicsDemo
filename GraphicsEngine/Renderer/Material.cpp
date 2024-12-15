#include "pch.h"
#include "Material.h"
#include "Device.h"
#include "BufferData.h"

Material::Material(const std::shared_ptr<Device>& device) : m_Device(device), m_AlbedoSRV(), m_NormalSRV(), m_MetalicSRV(), m_RoughnessSRV(), m_AOSRV(),m_Data()
{
}

Material::~Material()
{

}
