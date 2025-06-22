#ifndef CAMERA_H
#define CAMERA_H

#include "EngineCommon/util/Util.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/util/ImGuiSettings.h"

#include <functional>
#include <vector>

namespace engine
{

enum class MovementDirection : u8
{
    ViewDirection,
    SideDirection,
    TopDirection
};

// TODO: make a client camera with all client specific code
class Camera
{
public:
    using CallbackSign = std::function<void(const Camera* camera)>;
    Camera();
    void initialize(math::Vector3 pos,
                    math::Vector3 viewDirection, math::ProjectionProps props);
    void initialize(math::Vector3 pos, math::Vector3 viewDirection);

    math::Matrix4 getViewMatrix() const { return m_viewMatrix; }
    math::Matrix4 getProjectionMatrix() const { return m_projectionMatrix; }
    math::Vector3 getPos() const { return pos; }

    math::Vector3 getViewDir() const { return viewDir; }
    math::Vector3 getRightDir() const { return rightDir; }
    math::Vector3 getUpDir() const { return upDir; }

    void setPosition(math::Vector3 pos);
    void setDirection(math::Vector3 dir);

    void addChangeCallback(CallbackSign ptr) { m_callbacks.push_back(ptr); }
    void translate(MovementDirection direction, float velocity);
    void rotate(float vertical, float horizontal);
    void setProjectionProperties(math::ProjectionProps props)
    {
        updateProjectionMatrix(props);
    }

    void updateViewMatrix();
    void updateProjectionMatrix(math::ProjectionProps props);
    void update();
    void reset();
    void processUpdate();
    void setCameraMovementSpeed(float speed) { m_cameraMovementSpeed = speed; }
    void setCameraRotationSpeed(float speed) { m_cameraRotationSpeed = speed; }

private:
    static constexpr float defaultCameraMovementSpeed = 0.2f;
    static constexpr float defaultCameraRotationSpeed = .5f;
    math::Vector3 pos;
    math::Vector3 viewDir {0.f, 0.f, 1.f};
    math::Vector3 rightDir{1.f, 0.f, 0.f};
    math::Vector3 upDir   {0.f, 1.f, 0.f};

    math::Matrix4 m_viewMatrix;
    math::Matrix4 m_rotationMatrix;
    math::Matrix4 m_projectionMatrix;
    std::vector<CallbackSign> m_callbacks;
    float m_cameraMovementSpeed = defaultCameraMovementSpeed;
    util::UI_Float<Camera> m_cameraMovementSpeedButton;
    float m_cameraRotationSpeed = defaultCameraRotationSpeed;
    util::UI_Float<Camera> m_cameraRotationSpeedButton;

    bool m_needUpdate = false;
};

};

#endif // CAMERA_H
