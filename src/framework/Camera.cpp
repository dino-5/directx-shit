//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "include/Camera.h"
#include "include/ImGuiSettings.h"
#include <chrono>
#include <ctime>
#include <sys/utime.h>

using namespace DirectX;

float to_radians(float angle)
{
	return angle * MathHelper::Pi / 180;
}

Camera::Camera()
{
	SetLens(0.25f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{
}

XMVECTOR Camera::GetPosition()const
{
	XMVECTOR result{ mPosition[0], mPosition[1], mPosition[2], 1.0f};
	return result;
}

const float* Camera::GetPosition3f()const
{
	return mPosition;
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition[0] = x;
	mPosition[1] = y;
	mPosition[2] = z;

	mViewDirty = true;
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition[0] = v.x;
	mPosition[1] = v.y;
	mPosition[2] = v.z;
	mViewDirty = true;
}

XMVECTOR Camera::GetRight()const
{
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 Camera::GetRight3f()const
{
	return mRight;
}

XMVECTOR Camera::GetUp()const
{
	return XMLoadFloat3(&mUp);
}

XMFLOAT3 Camera::GetUp3f()const
{
	return mUp;
}

XMVECTOR Camera::GetLook()const
{
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 Camera::GetLook3f()const
{
	return mLook;
}

float Camera::GetNearZ()const
{
	return mNearZ;
}

float Camera::GetFarZ()const
{
	return mFarZ;
}

float Camera::GetAspect()const
{
	return mAspect;
}

float Camera::GetFovY()const
{
	return mFovY;
}

float Camera::GetFovX()const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atan(halfWidth / mNearZ);
}

float Camera::GetNearWindowWidth()const
{
	return mAspect * mNearWindowHeight;
}

float Camera::GetNearWindowHeight()const
{
	return mNearWindowHeight;
}

float Camera::GetFarWindowWidth()const
{
	return mAspect * mFarWindowHeight;
}

float Camera::GetFarWindowHeight()const
{
	return mFarWindowHeight;
}

XMVECTOR Camera::GetVector(DirectX::XMFLOAT3 fl)
{
	XMVECTOR res{0.f, 0.f, 0.f};
	XMStoreFloat3(&fl, res);
	return res;
}

void Camera::OnKeyDown(Key key)
{
	static time_t lastTimeSwitch = 0;
	XMVECTOR position{ mPosition[0], mPosition[1], mPosition[2]};
	XMVECTOR look{ mLook.x, mLook.y, mLook.z };
	XMVECTOR y{0.0, 1.0, 0.0};
	switch (key)
	{
	case Key::W:
			position += vel * look;
		break;
	case Key::A:
			position += XMVector3Cross(look, y) * vel;
		break;
	case Key::S:
			position -= vel * look;
		break;
	case Key::D:
			position -= XMVector3Cross(look, y) * vel;
		break;

	default:
		break;
	}
	mViewDirty = true;
	DirectX::XMFLOAT3 Position;
	XMStoreFloat3(&Position, position);
	mPosition[0] = Position.x;
	mPosition[1] = Position.y;
	mPosition[2] = Position.z;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf( 0.5f*mFovY );
	mFarWindowHeight  = 2.0f * mFarZ * tanf( 0.5f*mFovY );

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}

void Camera::SetWindowSize(float width, float height)
{
	m_height= height;
	m_width= width;
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	DirectX::XMFLOAT3 Position;
	XMStoreFloat3(&Position, pos);
	mPosition[0] = Position.x;
	mPosition[1] = Position.y;
	mPosition[2] = Position.z;
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);

	mViewDirty = true;
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	mViewDirty = true;
}

XMMATRIX Camera::GetView()
{
	//assert(!mViewDirty);
	if (mViewDirty)
		UpdateViewMatrix();
	return XMLoadFloat4x4(&mView);
}

void Camera::OnImGui()
{
	ImGuiSettings::Begin("Camera Position");
	{
		ImGuiSettings::SliderFloat3("pos", mPosition, -50, 50);
	}
	ImGuiSettings::End();
}

XMMATRIX Camera::GetProj()const
{
	return XMLoadFloat4x4(&mProj);
}


XMFLOAT4X4 Camera::GetView4x4f()const
{
	assert(!mViewDirty);
	return mView;
}

XMFLOAT4X4 Camera::GetProj4x4f()const
{
	return mProj;
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp,   XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	mViewDirty = true;
}

void Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight,   XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	mViewDirty = true;
}

XMMATRIX Camera::LookAt(XMFLOAT4 pos, XMFLOAT4 dir)
{
	XMVECTOR L = XMVector4Normalize(XMLoadFloat4(&dir));
	XMVECTOR P = XMLoadFloat4(&pos);
	XMVECTOR y{ 0.0f, 1.0f, 0.0f , 0.0f };
	XMVECTOR R = -XMVector3Cross(L, y);
	XMVECTOR U = XMVector3Cross(L, R);
	return XMMatrixLookAtLH(P, P + L, U);
}
XMMATRIX Camera::LookAt(XMFLOAT3 pos , XMFLOAT3 dir)
{
	XMVECTOR L = XMVector3Normalize(XMLoadFloat3(&dir));
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR y{ 0.0f, 1.0f, 0.0f };
	XMVECTOR R = XMVector3Cross(L, y);
	XMVECTOR U = XMVector3Cross(L, R);
	return XMMatrixLookAtLH(P, P + L, U);
}

XMMATRIX Camera::Ortho()
{
	return XMMatrixOrthographicLH(20, 20, 0.0f, 1000.0f);
	return XMMatrixOrthographicOffCenterLH(0, m_width, 0, m_height, 1.0f, 1000.0f);
}

void Camera::UpdateViewMatrix()
{
	if(mViewDirty)
	{
		XMVECTOR U = XMLoadFloat3(&mUp);
		mLook.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		mLook.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		mLook.y = sin(to_radians(m_pitch));
		XMVECTOR L = XMLoadFloat3(&mLook);
		DirectX::XMFLOAT3 Position(mPosition);
		XMVECTOR P = XMLoadFloat3(&Position);
		auto matrix = XMMatrixLookAtLH(P, P + L, U);
		XMStoreFloat4x4(&mView, matrix);
		mViewDirty = false;
	}
}

void Camera::OnMouseMove(int xpos, int ypos, bool update)
{
	static bool first = true;
	static double lastx;
	static double lasty;
	if (first)
	{
		lastx = xpos;
		lasty = ypos;
		first = false;
	}
	float dx = lastx - xpos;
	float dy = lasty - ypos;
	const float minCameraVelocity = 0.9;
	if (xpos<0)
		dx = minCameraVelocity;
	else if(xpos > m_width)
	{
		dx = -minCameraVelocity;
	}
	float sens = 0.1f;
	lastx = xpos;
	lasty = ypos;
	dx *= sens;
	dy *= sens;
	if (m_cameraOn) // add disable of move camera
	{
		m_yaw -= dx;
		m_pitch -= dy;
		mViewDirty = true;
	}
	else
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;
}
