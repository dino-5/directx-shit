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
	XMVECTOR result{ mPosition.x, mPosition.y, mPosition.z, 1.0f };
	return result;
}

XMFLOAT3 Camera::GetPosition3f()const
{
	return mPosition;
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
	mViewDirty = true;
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
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
	XMVECTOR position{ mPosition.x, mPosition.y, mPosition.z };
	XMVECTOR look{ mLook.x, mLook.y, mLook.z };
	XMVECTOR y{0.0, 1.0, 0.0};
	switch (key)
	{
	case Key::W:
		if(m_cameraOn)
			position += vel * look;
		break;
	case Key::A:
		if(m_cameraOn)
			position += XMVector3Cross(look, y) * vel;
		break;
	case Key::S:
		if(m_cameraOn)
			position -= vel * look;
		break;
	case Key::D:
		if(m_cameraOn)
			position -= XMVector3Cross(look, y) * vel;
		break;
	case Key::SWITCH_CAMERA:
		if (time(nullptr) - lastTimeSwitch > 1)
		{
			m_cameraOn = !m_cameraOn;
			lastTimeSwitch = time(nullptr);
		}

	default:
		break;
	}
	mViewDirty = true;
	XMStoreFloat3(&mPosition, position);
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

	XMStoreFloat3(&mPosition, pos);
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
	ImGui::Begin("Camera Position");
	{
		ImGui::SliderFloat3("pos", &mPosition.x, -50, 50);
	}
	ImGui::End();
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

void Camera::UpdateViewMatrix()
{
	if(mViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		mLook.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		mLook.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		mLook.y = sin(to_radians(m_pitch));
		XMVECTOR L = XMLoadFloat3(&mLook);
		XMVECTOR P = XMLoadFloat3(&mPosition);
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
	float sens = 0.1f;
	lastx = xpos;
	lasty = ypos;
	dx *= sens;
	dy *= sens;
	if (m_cameraOn) // add disable of move camera
	{
		m_yaw += dx;
		m_pitch += dy;
		mViewDirty = true;
		//Pitch(m_pitch);
		//RotateY(m_yaw);
	}
	else
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;
}


