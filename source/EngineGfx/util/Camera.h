#ifndef CAMERA_H
#define CAMERA_H

#include "EngineCommon/util/Util.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/math/Vector.h"

#include <functional>
#include <vector>

namespace engine::graphics
{

	class Camera
	{
	public:
		Camera() = default;
		void Initialize(math::Vector3 pos, math::Vector3 viewDirection);
		math::Matrix4 GetViewMatrix() { return m_viewMatrix; }
		void Translate(const math::Vector3& offset);
		void AddChangeCallback(std::function<void()> ptr) { m_callbacks.push_back(ptr); }
		void Update();
	private:
		math::Vector3 m_position;
		math::Vector3 m_viewDir{0.f, 0.f, 1.f};
		math::Vector3 m_rightDir{1.f, 0.f, 0.f};
		math::Vector3 m_upDir{0.f, 1.f, 0.f};

		math::Matrix4 m_viewMatrix;
		std::vector<std::function<void()>> m_callbacks;
	};

};

#endif // CAMERA_H