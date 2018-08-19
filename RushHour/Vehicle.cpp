#include "stdafx.h"
#include "Vehicle.h"

using namespace std;
using namespace DirectX;

Vehicle::Vehicle(Model & m, DirectX::XMMATRIX scale, DirectX::XMVECTOR alignment, bool rotate180, unsigned short length) : ModelInstance(m, scale, alignment, XMMatrixIdentity()) {
	_length = length;
	_rotate180 = rotate180;
}

void Vehicle::SetMovementStep(float s) {
	_movementStep = s; 
}

DirectX::XMMATRIX Vehicle::GetTransformation() const {
	XMMATRIX r;
	XMMATRIX p = XMMatrixTranslation(static_cast<float>(GetPosition().x), 0.0f, static_cast<float>(GetPosition().z));
	if (GetOrientation() == Vehicle::XAxis) {
		// rotate the instance 90degrees
		r = XMMatrixRotationY(XMConvertToRadians(90.0f));
		// rotate the alignment and movementStep 90degrees
		p *= XMMatrixTranslation(GetAlignment().m128_f32[0], GetAlignment().m128_f32[1], GetAlignment().m128_f32[2]);
		p *= XMMatrixTranslation(_movementStep, 0.0f, 0.0f);
	}
	else {
		// no rotation
		r = XMMatrixIdentity();
		p *= XMMatrixTranslation(GetAlignment().m128_f32[2], GetAlignment().m128_f32[1], GetAlignment().m128_f32[0]);
		p *= XMMatrixTranslation(0.0f, 0.0f, _movementStep);
	}
	if (_rotate180) {
		r *= XMMatrixRotationY(XMConvertToRadians(180.0f));
	}
	return r * GetScale() * p;
}

// Return rotation matrix reflecting initial orientation of instance based on IsRotated180() and GetOrientation() state.
// It does not reflect world rotation.
DirectX::XMMATRIX Vehicle::GetInitRotation() const {
	XMMATRIX rMatrix = ModelInstance::GetInitRotation();
	if (IsRotated180()) {
		rMatrix *= XMMatrixRotationY(XMConvertToRadians(180));
	}
	return rMatrix;
}

// Returns true if this instance is occupying position pos
bool Vehicle::IsOccupyingPosition(Coords2i_t pos) const {
	if (IsHidden()) {
		return false;
	}
	Coords2i_t p = GetPosition();
	for (unsigned short i = 0; i < _length; i++) {
		if ((p.x == pos.x) && (p.z == pos.z)) {
			return true;
		}
		if (GetOrientation() == Vehicle::XAxis) {
			p.x += 1;
		} else {
			p.z += 1;
		}
	}
	return false;
}

