#ifndef CAMERA_H
#define CAMERA_H

#include "EngineCommon/util/Util.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/math/Vector.h"

#include <functional>
#include <vector>

namespace engine::graphics
{

	enum class MovementDirection : u8
	{
		ViewDirection,
		SideDirection
	};

	// TODO: make a client camera with all client specific code
	class Camera
	{
	public:
		using CallbackSign = std::function<void(const Camera* camera)>;
		Camera() = default;
		void initialize(math::Vector3 pos, math::Vector3 viewDirection);
		math::Matrix4 getViewMatrix() const { return m_viewMatrix; }
		void translate(MovementDirection direction, float velocity);
		void addChangeCallback(CallbackSign ptr) { m_callbacks.push_back(ptr); }
		void rotate(float vertical, float horizontal);
		void updateViewMatrix();
		void update();
		void reset();
		void processUpdate();

	private:
		math::Vector3 m_position;
		math::Vector3 m_viewDir {0.f, 0.f, 1.f};
		math::Vector3 m_rightDir{1.f, 0.f, 0.f};
		math::Vector3 m_upDir   {0.f, 1.f, 0.f};

		math::Matrix4 m_viewMatrix;
		math::Matrix4 m_rotationMatrix;
		std::vector<CallbackSign> m_callbacks;
	};

};

#endif // CAMERA_H