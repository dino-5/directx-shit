
#include "Camera.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include <chrono>
#include <ctime>
#include <sys/utime.h>

using namespace DirectX;
using namespace engine;

using namespace engine::graphics;

void Camera::Initialize(math::Vector3 pos, math::Vector3 viewDirection)
{
    m_viewDir = viewDirection;
    m_position = pos;
    m_viewMatrix = math::CreateViewMatrix(pos, m_viewDir, m_upDir, m_rightDir);
}

void Camera::Update()
{
    for (auto& callback : m_callbacks)
        callback();
}

void Camera::Translate(const math::Vector3& offset)
{
    m_position = m_position + offset;
    m_viewMatrix = m_viewMatrix * math::Translate(m_position);
    Update();
}
