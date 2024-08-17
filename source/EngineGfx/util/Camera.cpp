
#include "Camera.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/System/InputManager.h"
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
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    m_rotationMatrix = math::CreateViewRotationMatrix(m_viewDir, m_upDir, m_rightDir);
    m_viewMatrix = math::Translate(m_position) * m_rotationMatrix;
}

void Camera::Update()
{
    // TODO: optimize it to call it once per frame after all changes are done
    // TODO: move input handling in client specific implementation

    // TODO: frame independent movement https://gamedev.stackexchange.com/questions/9515/frame-independent-movement
	float velocity = .1f;
	float rotationVelocity = 2.f;

    auto inputManager = system::InputManager::GetInputManager();

    if(inputManager.GetKeyState(system::Key::D).IsPressed())
        Translate(graphics::MovementDirection::SideDirection, velocity);

    if(inputManager.GetKeyState(system::Key::W).IsPressed())
        Translate(graphics::MovementDirection::ViewDirection, velocity);

    if(inputManager.GetKeyState(system::Key::A).IsPressed())
        Translate(graphics::MovementDirection::SideDirection, -velocity);

    if(inputManager.GetKeyState(system::Key::S).IsPressed())
        Translate(graphics::MovementDirection::ViewDirection, -velocity);

    if(inputManager.GetKeyState(system::Key::UP).IsPressed())
        Rotate(-rotationVelocity, 0);

    if(inputManager.GetKeyState(system::Key::DOWN).IsPressed())
        Rotate(rotationVelocity, 0);

    if(inputManager.GetKeyState(system::Key::RIGHT).IsPressed())
        Rotate(0, rotationVelocity);

    if(inputManager.GetKeyState(system::Key::LEFT).IsPressed())
        Rotate(0, -rotationVelocity);

    if(inputManager.GetKeyState(system::Key::R).IsPressed())
        Reset();

    UpdateViewMatrix();
    ProcessUpdate();
}

void Camera::ProcessUpdate()
{
    for (auto& callback : m_callbacks)
        callback();
}

void Camera::Reset()
{
    m_position = {0.f, 0.f, 0.f};
    m_viewDir  = {0.f, 0.f, 1.f};
    m_rightDir = {1.f, 0.f, 0.f};
    m_upDir    = {0.f, 1.f, 0.f};
}

void Camera::Translate(MovementDirection direction, float velocity)
{
    auto offset = static_cast<math::Vector3*>(&m_viewDir)[(u8)direction] * velocity;
    m_position = m_position + offset;
}

/*
    vertical represent camera movement up and down, where up direction >0
    horizontal represent right and left, where right >0
*/
void Camera::Rotate(float vertical, float horizontal)
{
    math::Quartenion rotationY(m_upDir, horizontal);
    math::Quartenion rotationX(m_rightDir, vertical);
    auto rotationMatrixX = math::Matrix4(rotationX);
    auto rotationMatrixY = math::Matrix4(rotationY);
    auto rotationMatrixXY = rotationMatrixX * rotationMatrixY;

    m_viewDir = (rotationMatrixXY * math::Vector4(m_viewDir, { 1.f }));
    m_viewDir.NormalizeSelf();
    m_rightDir = (rotationMatrixY * math::Vector4(m_rightDir, { 1.f }));
    m_rightDir.NormalizeSelf();
    m_upDir = (rotationMatrixX * math::Vector4(m_upDir, { 1.f }));
    m_upDir.NormalizeSelf();
}
