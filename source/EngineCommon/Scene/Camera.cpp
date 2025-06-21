#include "Camera.h"
#include "EngineCommon/util/ImGuiSettings.h"
#include "EngineCommon/util/Timer.h"
#include "EngineCommon/System/InputManager.h"

using namespace DirectX;
using namespace engine;

using namespace engine::math;

Camera::Camera() 
    : m_cameraMovementSpeedButton(*this,
                                  defaultCameraMovementSpeed,
                                  "CameraMovementSpeed", 100),
      m_cameraRotationSpeedButton(*this,
                                  defaultCameraRotationSpeed,
                                  "CameraRotationSpeed", 5)
{
    m_cameraMovementSpeedButton.setCallback([](Camera& camera, float speed)
    {
        camera.setCameraMovementSpeed(speed);
    });
    m_cameraRotationSpeedButton.setCallback([](Camera& camera, float speed)
    {
        camera.setCameraRotationSpeed(speed);
    });
}

void Camera::initialize(math::Vector3 aPos, 
                        math::Vector3 viewDirection, 
                        math::ProjectionProps props)
{
	setDirection(viewDirection);
    pos = aPos;
    updateViewMatrix();
    updateProjectionMatrix(props);
}

void Camera::initialize(math::Vector3 aPos, math::Vector3 viewDirection)
{
	setDirection(viewDirection);
    pos = aPos;
    updateViewMatrix();
}

void Camera::setPosition(math::Vector3 aPos)
{
    pos = aPos;
}

void Camera::setDirection(math::Vector3 dir)
{
    viewDir = dir.normalize();
	math::Vector3 y{0.f, 1.f, 0.f};
	rightDir = math::CrossProduct(y, viewDir).normalize();
	upDir = math::CrossProduct(viewDir, rightDir).normalize();
}


void Camera::updateViewMatrix()
{
    m_rotationMatrix = math::CreateViewRotationMatrix(viewDir, upDir, rightDir);
    m_viewMatrix = math::Translate(-pos) * m_rotationMatrix;
}

void Camera::updateProjectionMatrix(math::ProjectionProps props)
{
    m_projectionMatrix = GetProjectionMatrix(props);
}

void Camera::update()
{
    // TODO: optimize it to call it once per frame after all changes are done
    // TODO: move input handling in client specific implementation

    float velocity = 2.f;
    float rotationVelocity = 5.f;
    static util::Timer timer("camera");
    float time = (float)timer.getElapsedTime();
    timer.saveCurrentTime();

    velocity *= (float)time * m_cameraMovementSpeed * 0.1;
    rotationVelocity *= (float)time * m_cameraRotationSpeed * 0.1;

    auto inputManager = system::InputManager::GetInputManager();

    if (inputManager.getKeyState(system::Key::D).isPressed())
        translate(MovementDirection::SideDirection, velocity);

    if(inputManager.getKeyState(system::Key::A).isPressed())
        translate(MovementDirection::SideDirection, -velocity);

    if(inputManager.getKeyState(system::Key::W).isPressed())
        translate(MovementDirection::ViewDirection, velocity);

    if(inputManager.getKeyState(system::Key::S).isPressed())
        translate(MovementDirection::ViewDirection, -velocity);

    if(inputManager.getKeyState(system::Key::E).isPressed())
        translate(MovementDirection::TopDirection, velocity);

    if(inputManager.getKeyState(system::Key::Q).isPressed())
        translate(MovementDirection::TopDirection, -velocity);

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
    pos = {0.f, 0.f, 0.f};
    viewDir  = {0.f, 0.f, 1.f};
    rightDir = {1.f, 0.f, 0.f};
    upDir    = {0.f, 1.f, 0.f};
    m_needUpdate = true;
}

void Camera::translate(MovementDirection direction, float velocity)
{
    auto offset = (&viewDir)[(u8)direction] * velocity;
    pos = pos + offset;
    m_needUpdate = true;
}

/*
    vertical represent camera movement up and down, where up direction >0
    horizontal represent right and left, where right >0
*/
void Camera::rotate(float vertical, float horizontal)
{
    math::Quartenion rotationY(upDir, horizontal);
    math::Quartenion rotationX(rightDir, vertical);
    auto rotationMatrixX = math::Matrix4(rotationX);
    auto rotationMatrixY = math::Matrix4(rotationY);
    auto rotationMatrixXY = rotationMatrixX * rotationMatrixY;

    viewDir = (rotationMatrixXY * Vector4(viewDir, { 1.f }));
    viewDir.normalizeSelf();
    rightDir = (rotationMatrixY * Vector4(rightDir, { 1.f }));
    rightDir.normalizeSelf();
    upDir = (rotationMatrixX * math::Vector4(upDir, { 1.f }));
    upDir.normalizeSelf();
    m_needUpdate = true;
}
