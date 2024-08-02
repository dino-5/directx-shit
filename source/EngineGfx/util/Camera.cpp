
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
    m_rotationMatrix = math::CreateViewRotationMatrix(m_viewDir, m_upDir, m_rightDir);
    m_viewMatrix = math::Translate(pos) * m_rotationMatrix;
}

void Camera::Update()
{
    for (auto& callback : m_callbacks)
        callback();
}

void Camera::Translate(const math::Vector3& offset)
{
    m_position = m_position + offset;
    m_viewMatrix =  math::Translate(m_position) * m_rotationMatrix;
    Update();
}

/*
    vertical represent camera movement up and down, where up direction >0
    horizontal represent right and left, where right >0
*/
void Camera::Rotate(float vertical, float horizontal)
{
        
}
