#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera() {
	_camPosition = CAMINITPOSITION;
}


Camera::~Camera() {
}

void Camera::SetNewPosition(XMMATRIX rotation) {
	_camPosition = CAMINITPOSITION;
	_camPosition = XMVector4Transform(_camPosition, rotation);
}

DirectX::XMMATRIX Camera::GetViewMatrix() const {
	return XMMatrixLookAtLH(
		_camPosition,                         // the camera position (rotating around the center of the board)
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // the look-at position
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // the up direction
	);
}
