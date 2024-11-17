
#include "Camera.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/System/InputManager.h"
#include <chrono>
#include <ctime>
#include <sys/utime.h>

using namespace DirectX;
using namespace engine;

using namespace engine::graphics;

void Camera::initialize(math::Vector3 pos, math::Vector3 viewDirection)
{
    m_viewDir = viewDirection;
    m_position = pos;
    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    m_rotationMatrix = math::CreateViewRotationMatrix(m_viewDir, m_upDir, m_rightDir);
    m_viewMatrix = math::Translate(m_position) * m_rotationMatrix;
}

void Camera::update()
{
    // TODO: optimize it to call it once per frame after all changes are done
    // TODO: move input handling in client specific implementation

    // TODO: frame independent movement https://gamedev.stackexchange.com/questions/9515/frame-independent-movement
    float velocity = 2.f;
    float rotationVelocity = 5.f;

    auto inputManager = system::InputManager::GetInputManager();

    if (inputManager.getKeyState(system::Key::D).isPressed())
        translate(graphics::MovementDirection::SideDirection, velocity);

    if(inputManager.getKeyState(system::Key::W).isPressed())
        translate(graphics::MovementDirection::ViewDirection, velocity);

    if(inputManager.getKeyState(system::Key::A).isPressed())
        translate(graphics::MovementDirection::SideDirection, -velocity);

    if(inputManager.getKeyState(system::Key::S).isPressed())
        translate(graphics::MovementDirection::ViewDirection, -velocity);

    if(inputManager.getKeyState(system::Key::UP).isPressed())
        rotate(-rotationVelocity, 0);

    if(inputManager.getKeyState(system::Key::DOWN).isPressed())
        rotate(rotationVelocity, 0);

    if(inputManager.getKeyState(system::Key::RIGHT).isPressed())
        rotate(0, rotationVelocity);

    if(inputManager.getKeyState(system::Key::LEFT).isPressed())
        rotate(0, -rotationVelocity);

    if(inputManager.getKeyState(system::Key::R).isPressed())
        reset();

    if (m_needUpdate)
    {
        updateViewMatrix();
        processUpdate();
        m_needUpdate = false;
    }
}

void Camera::processUpdate()
{
    for (auto& callback : m_callbacks)
        callback(this);
}

void Camera::reset()
{
    m_position = {0.f, 0.f, 0.f};
    m_viewDir  = {0.f, 0.f, 1.f};
    m_rightDir = {1.f, 0.f, 0.f};
    m_upDir    = {0.f, 1.f, 0.f};
    m_needUpdate = true;
}

void Camera::translate(MovementDirection direction, float velocity)
{
    auto offset = static_cast<math::Vector3*>(&m_viewDir)[(u8)direction] * velocity;
    m_position = m_position + offset;
    m_needUpdate = true;
}

/*
    vertical represent camera movement up and down, where up direction >0
    horizontal represent right and left, where right >0
*/
void Camera::rotate(float vertical, float horizontal)
{
    math::Quartenion rotationY(m_upDir, horizontal);
    math::Quartenion rotationX(m_rightDir, vertical);
    auto rotationMatrixX = math::Matrix4(rotationX);
    auto rotationMatrixY = math::Matrix4(rotationY);
    auto rotationMatrixXY = rotationMatrixX * rotationMatrixY;

    m_viewDir = (rotationMatrixXY * math::Vector4(m_viewDir, { 1.f }));
    m_viewDir.normalizeSelf();
    m_rightDir = (rotationMatrixY * math::Vector4(m_rightDir, { 1.f }));
    m_rightDir.normalizeSelf();
    m_upDir = (rotationMatrixX * math::Vector4(m_upDir, { 1.f }));
    m_upDir.normalizeSelf();
    m_needUpdate = true;
}
