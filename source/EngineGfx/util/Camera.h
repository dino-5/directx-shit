#ifndef CAMERA_H
#define CAMERA_H

#include "EngineCommon/util/Util.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/math/Vector.h"

namespace engine::graphics
{

	class Camera
	{
	public:
		Camera() = default;
		void Initialize(math::Vector4 pos, math::Vector3 viewDirection);
		math::Matrix4 GetViewMatrix() { return m_view; }
	private:
		math::Matrix4 m_view;
	};

};

#endif // CAMERA_H